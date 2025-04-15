#include "BasicGraphicsScene.hpp"

#include "AbstractNodeGeometry.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "ConnectionIdUtils.hpp"
#include "DefaultConnectionPainter.hpp"
#include "DefaultHorizontalNodeGeometry.hpp"
#include "DefaultNodePainter.hpp"
#include "DefaultVerticalNodeGeometry.hpp"
#include "GraphicsView.hpp"
#include "NodeGraphicsObject.hpp"

#include <QUndoStack>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGraphicsSceneMoveEvent>

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QtGlobal>

#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <queue>

namespace QtNodes {

BasicGraphicsScene::BasicGraphicsScene(AbstractGraphModel &graphModel, QObject *parent)
    : QGraphicsScene(parent)
    , _graphModel(graphModel)
    , _nodeGeometry(std::make_unique<DefaultHorizontalNodeGeometry>(_graphModel))
    , _nodePainter(std::make_unique<DefaultNodePainter>())
    , _connectionPainter(std::make_unique<DefaultConnectionPainter>())
    , _nodeDrag(false)
    , _undoStack(new QUndoStack(this))
    , _orientation(Qt::Horizontal)
{
    // Disables the indexing for performance in large scenes.
    setItemIndexMethod(QGraphicsScene::NoIndex);

    connect(&_graphModel,
            &AbstractGraphModel::connectionCreated,
            this,
            &BasicGraphicsScene::onConnectionCreated);

    connect(&_graphModel,
            &AbstractGraphModel::connectionDeleted,
            this,
            &BasicGraphicsScene::onConnectionDeleted);

    connect(&_graphModel,
            &AbstractGraphModel::nodeCreated,
            this,
            &BasicGraphicsScene::onNodeCreated);

    connect(&_graphModel,
            &AbstractGraphModel::nodeDeleted,
            this,
            &BasicGraphicsScene::onNodeDeleted);

    connect(&_graphModel,
            &AbstractGraphModel::nodePositionUpdated,
            this,
            &BasicGraphicsScene::onNodePositionUpdated);

    connect(&_graphModel,
            &AbstractGraphModel::nodeUpdated,
            this,
            &BasicGraphicsScene::onNodeUpdated);

    connect(this, &BasicGraphicsScene::nodeClicked, this, &BasicGraphicsScene::onNodeClicked);

    connect(&_graphModel, &AbstractGraphModel::modelReset, this, &BasicGraphicsScene::onModelReset);

    traverseGraphAndPopulateGraphicsObjects();
}

BasicGraphicsScene::~BasicGraphicsScene() = default;

AbstractGraphModel const &BasicGraphicsScene::graphModel() const
{
    return _graphModel;
}

AbstractGraphModel &BasicGraphicsScene::graphModel()
{
    return _graphModel;
}

AbstractNodeGeometry &BasicGraphicsScene::nodeGeometry()
{
    return *_nodeGeometry;
}

AbstractNodePainter &BasicGraphicsScene::nodePainter()
{
    return *_nodePainter;
}

AbstractConnectionPainter &BasicGraphicsScene::connectionPainter()
{
    return *_connectionPainter;
}

void BasicGraphicsScene::setNodePainter(std::unique_ptr<AbstractNodePainter> newPainter)
{
    _nodePainter = std::move(newPainter);
}

void BasicGraphicsScene::setConnectionPainter(std::unique_ptr<AbstractConnectionPainter> newPainter)
{
    _connectionPainter = std::move(newPainter);
}

QUndoStack &BasicGraphicsScene::undoStack()
{
    return *_undoStack;
}

// Starts a draft connection (user is dragging a wire).
// Grabs mouse input to track movement.

QImage BasicGraphicsScene::cvMatToQImage(const cv::Mat& mat) const
{
    switch (mat.type()) {
        case CV_8UC3: {
            QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return img.rgbSwapped();
        }
        case CV_8UC1: {
            QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
            return img.copy();
        }
        case CV_8UC4: {
            QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return img.copy();
        }
        default:
            // Convert other types to 8UC3 then recurse
            cv::Mat tmp;
            mat.convertTo(tmp, CV_8UC3);
            return cvMatToQImage(tmp);
    }
}

std::unique_ptr<ConnectionGraphicsObject> const &BasicGraphicsScene::makeDraftConnection(
    ConnectionId const incompleteConnectionId)
{
    _draftConnection = std::make_unique<ConnectionGraphicsObject>(*this, incompleteConnectionId);

    _draftConnection->grabMouse();
//
    return _draftConnection;
}
//Cancels the draft connection.
void BasicGraphicsScene::resetDraftConnection()
{
    _draftConnection.reset();
}

//Deletes all nodes in the model, which also deletes connections.


void BasicGraphicsScene::clearScene()
{
    auto const &allNodeIds = graphModel().allNodeIds();

    for (auto nodeId : allNodeIds) {
        graphModel().deleteNode(nodeId);
    }
}

NodeGraphicsObject *BasicGraphicsScene::nodeGraphicsObject(NodeId nodeId)
{
    NodeGraphicsObject *ngo = nullptr;
    auto it = _nodeGraphicsObjects.find(nodeId);
    if (it != _nodeGraphicsObjects.end()) {
        ngo = it->second.get();
    }

    return ngo;
}

ConnectionGraphicsObject *BasicGraphicsScene::connectionGraphicsObject(ConnectionId connectionId)
{
    ConnectionGraphicsObject *cgo = nullptr;
    auto it = _connectionGraphicsObjects.find(connectionId);
    if (it != _connectionGraphicsObjects.end()) {
        cgo = it->second.get();
    }

    return cgo;
}
//Changes node orientation (horizontal/vertical), replaces node geometry and resets the model (to redraw with new layout).


void BasicGraphicsScene::setOrientation(Qt::Orientation const orientation)
{
    if (_orientation != orientation) {
        _orientation = orientation;

        switch (_orientation) {
        case Qt::Horizontal:
            _nodeGeometry = std::make_unique<DefaultHorizontalNodeGeometry>(_graphModel);
            break;

        case Qt::Vertical:
            _nodeGeometry = std::make_unique<DefaultVerticalNodeGeometry>(_graphModel);
            break;
        }

        onModelReset();
    }
}
//Stub for a context menu. Returns nullptr by default.
QMenu *BasicGraphicsScene::createSceneMenu(QPointF const scenePos)
{
    Q_UNUSED(scenePos);
    return nullptr;
}

void BasicGraphicsScene::traverseGraphAndPopulateGraphicsObjects()
{
    auto allNodeIds = _graphModel.allNodeIds();

    // First create all the nodes.
    for (NodeId const nodeId : allNodeIds) {
        _nodeGraphicsObjects[nodeId] = std::make_unique<NodeGraphicsObject>(*this, nodeId);
    }

    // Then for each node check output connections and insert them.
    for (NodeId const nodeId : allNodeIds) {
        auto nOutPorts = _graphModel.nodeData<PortCount>(nodeId, NodeRole::OutPortCount);

        for (PortIndex index = 0; index < nOutPorts; ++index) {
            auto const &outConnectionIds = _graphModel.connections(nodeId, PortType::Out, index);

            for (auto cid : outConnectionIds) {
                _connectionGraphicsObjects[cid] = std::make_unique<ConnectionGraphicsObject>(*this,
                                                                                             cid);
            }
        }
    }
}

//Refreshes the node connected at a given end of a connection
void BasicGraphicsScene::updateAttachedNodes(ConnectionId const connectionId,
                                             PortType const portType)
{
    auto node = nodeGraphicsObject(getNodeId(portType, connectionId));

    if (node) {
        node->update();
    }
}

//removes visual wire
void BasicGraphicsScene::onConnectionDeleted(ConnectionId const connectionId)
{
    auto it = _connectionGraphicsObjects.find(connectionId);
    if (it != _connectionGraphicsObjects.end()) {
        _connectionGraphicsObjects.erase(it);
    }

    // TODO: do we need it?
    if (_draftConnection && _draftConnection->connectionId() == connectionId) {
        _draftConnection.reset();
    }

    updateAttachedNodes(connectionId, PortType::Out);
    updateAttachedNodes(connectionId, PortType::In);

    Q_EMIT modified(this);
}

// adds visual wire
void BasicGraphicsScene::onConnectionCreated(ConnectionId const connectionId)
{
    _connectionGraphicsObjects[connectionId]
        = std::make_unique<ConnectionGraphicsObject>(*this, connectionId);

    updateAttachedNodes(connectionId, PortType::Out);
    updateAttachedNodes(connectionId, PortType::In);

    Q_EMIT modified(this);
}

//removes visual node

void BasicGraphicsScene::onNodeDeleted(NodeId const nodeId)
{
    auto it = _nodeGraphicsObjects.find(nodeId);
    if (it != _nodeGraphicsObjects.end()) {
        _nodeGraphicsObjects.erase(it);

        Q_EMIT modified(this);
    }
}

//adds visual node
// void BasicGraphicsScene::onNodeCreated(NodeId const nodeId)
// {
//     _nodeGraphicsObjects[nodeId] = std::make_unique<NodeGraphicsObject>(*this, nodeId);

//     Q_EMIT modified(this);
// }
void BasicGraphicsScene::onNodeCreated(NodeId const nodeId)
{
    // Create the visual representation of the node.
    _nodeGraphicsObjects[nodeId] = std::make_unique<NodeGraphicsObject>(*this, nodeId);

    // Get the NodeGraphicsObject for the newly created node.
    NodeGraphicsObject *nodeGraphicsObject = this->nodeGraphicsObject(nodeId);
    if (nodeGraphicsObject) {
        // Connect the nodeBodyClicked signal to a lambda or slot for handling image loading.
        connect(nodeGraphicsObject, &NodeGraphicsObject::nodeBodyClicked, [nodeId,this]()  {
            // Open a file dialog to load an image for the clicked node.
            openImageFileDialog(nodeId);
        });
    }

    Q_EMIT modified(this);
}

cv::Mat BasicGraphicsScene::loadImage(const QString &filePath) const {
    cv::Mat image = cv::imread(filePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        qWarning() << "Failed to load image:" << filePath;
    }
    return image;
}

// void BasicGraphicsScene::openImageFileDialog(NodeId nodeId)
// {
//     // Open a file dialog to select an image file.
//     QString filePath = QFileDialog::getOpenFileName(
//         nullptr, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");

//     if (!filePath.isEmpty()) {
//         // Load the image using OpenCV or Qt.
//         QImage image(filePath);
//         if (image.isNull()) {
//             // Handle invalid image file.
//             qWarning() << "Failed to load image:" << filePath;
//             return;
//         }

//         // Store the image data in the model (e.g., as a QPixmap).
//         graphModel().setNodeData(nodeId, NodeRole::InternalData, QVariant::fromValue(QPixmap::fromImage(image)));

//         // Trigger a repaint of the node to display the image.
//         auto nodeGraphicsObject = this->nodeGraphicsObject(nodeId);
//         if (nodeGraphicsObject) {
//             nodeGraphicsObject->update();//// Trigger repaint to display the image.
//         }
//     }
// }
void BasicGraphicsScene::openImageFileDialog(NodeId nodeId)
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!filePath.isEmpty()) {
        // Load the image using OpenCV
        cv::Mat mat = loadImage(filePath); // Call the loadImage function
        if (mat.empty()) {
            return; // Exit if the image failed to load
        }

        // Convert the OpenCV Mat to QImage
        QImage image = cvMatToQImage(mat);

        // Wrap the QImage in a QPixmap for storage
        QPixmap pixmap = QPixmap::fromImage(image);

        // Store the QPixmap in the graph model
        graphModel().setNodeData(nodeId, NodeRole::InternalData, QVariant::fromValue(pixmap));

        // Trigger a repaint of the node
        auto nodeGraphicsObject = this->nodeGraphicsObject(nodeId);
        if (nodeGraphicsObject) {
            nodeGraphicsObject->update();
        }
    }
}
//updates position in view
void BasicGraphicsScene::onNodePositionUpdated(NodeId const nodeId)
{
    auto node = nodeGraphicsObject(nodeId);
    if (node) {
        node->setPos(_graphModel.nodeData(nodeId, NodeRole::Position).value<QPointF>());
        node->update();
        _nodeDrag = true;
    }
}
//resizes, moves, updates widget and connections
void BasicGraphicsScene::onNodeUpdated(NodeId const nodeId)
{
    auto node = nodeGraphicsObject(nodeId);

    if (node) {
        node->setGeometryChanged();

        _nodeGeometry->recomputeSize(nodeId);

        node->updateQWidgetEmbedPos();
        node->update();
        node->moveConnections();
    }
}
// emits signal if a node was dragged
void BasicGraphicsScene::onNodeClicked(NodeId const nodeId)
{
    if (_nodeDrag) {
        Q_EMIT nodeMoved(nodeId, _graphModel.nodeData(nodeId, NodeRole::Position).value<QPointF>());
        Q_EMIT modified(this);
    }
    _nodeDrag = false;
}
//clears and repopulates everything
void BasicGraphicsScene::onModelReset()
{
    _connectionGraphicsObjects.clear();
    _nodeGraphicsObjects.clear();

    clear();

    traverseGraphAndPopulateGraphicsObjects();
}

} // namespace QtNodes

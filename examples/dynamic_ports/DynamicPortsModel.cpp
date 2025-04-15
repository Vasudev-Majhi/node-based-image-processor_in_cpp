#include "DynamicPortsModel.hpp"

#include "PortAddRemoveWidget.hpp"

#include <QtNodes/ConnectionIdUtils>

#include <QJsonArray>

#include <QBuffer>
#include <iterator>

#include <QFileDialog>
DynamicPortsModel::DynamicPortsModel()
    : _nextNodeId{0}
{}

DynamicPortsModel::~DynamicPortsModel()
{
    //
}

std::unordered_set<NodeId> DynamicPortsModel::allNodeIds() const
{
    return _nodeIds;
}

std::unordered_set<ConnectionId> DynamicPortsModel::allConnectionIds(NodeId const nodeId) const
{
    std::unordered_set<ConnectionId> result;

    std::copy_if(_connectivity.begin(),
                 _connectivity.end(),
                 std::inserter(result, std::end(result)),
                 [&nodeId](ConnectionId const &cid) {
                     return cid.inNodeId == nodeId || cid.outNodeId == nodeId;
                 });

    return result;
}

std::unordered_set<ConnectionId> DynamicPortsModel::connections(NodeId nodeId,
                                                                PortType portType,
                                                                PortIndex portIndex) const
{
    std::unordered_set<ConnectionId> result;

    std::copy_if(_connectivity.begin(),
                 _connectivity.end(),
                 std::inserter(result, std::end(result)),
                 [&portType, &portIndex, &nodeId](ConnectionId const &cid) {
                     return (getNodeId(portType, cid) == nodeId
                             && getPortIndex(portType, cid) == portIndex);
                 });

    return result;
}QImage cvMatToQImage(const cv::Mat& mat)
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
        default: {
            cv::Mat tmp;
            mat.convertTo(tmp, CV_8UC3);
            return cvMatToQImage(tmp);
        }
    }
}

// cv::Mat QImageToCvMat(const QImage& image)
// {
//     switch (image.format()) {
//         case QImage::Format_RGB32:
//         case QImage::Format_ARGB32:
//         case QImage::Format_ARGB32_Premultiplied:
//             return cv::Mat(image.height(),
//                            image.width(),
//                            CV_8UC4,
//                            const_cast<uchar*>(image.bits()),
//                            static_cast<size_t>(image.bytesPerLine()));
//         case QImage::Format_RGB888:
//             return cv::Mat(image.height(),
//                            image.width(),
//                            CV_8UC3,
//                            const_cast<uchar*>(image.bits()),
//                            static_cast<size_t>(image.bytesPerLine()));
//         case QImage::Format_Grayscale8:
//             return cv::Mat(image.height(),
//                            image.width(),
//                            CV_8UC1,
//                            const_cast<uchar*>(image.bits()),
//                            static_cast<size_t>(image.bytesPerLine()));
//         default: {
//             QImage converted = image.convertToFormat(QImage::Format_RGB888);
//             return QImageToCvMat(converted);
//         }
//     }
// }
cv::Mat QImageToCvMat(const QImage& image)
{
    switch (image.format()) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            return cv::Mat(image.height(),
                           image.width(),
                           CV_8UC4,
                           const_cast<uchar*>(image.bits()),
                           static_cast<size_t>(image.bytesPerLine()));
        case QImage::Format_RGB888:
            return cv::Mat(image.height(),
                           image.width(),
                           CV_8UC3,
                           const_cast<uchar*>(image.bits()),
                           static_cast<size_t>(image.bytesPerLine()));
        case QImage::Format_Grayscale8:
            return cv::Mat(image.height(),
                           image.width(),
                           CV_8UC1,
                           const_cast<uchar*>(image.bits()),
                           static_cast<size_t>(image.bytesPerLine()));
        default: {
            QImage converted = image.convertToFormat(QImage::Format_RGB888);
            return QImageToCvMat(converted);
        }
    }
}
bool DynamicPortsModel::connectionExists(ConnectionId const connectionId) const
{
    return (_connectivity.find(connectionId) != _connectivity.end());
}

NodeId DynamicPortsModel::addNode(QString const nodeType)
{
    NodeId newId = newNodeId();

    // Create new node.
    _nodeIds.insert(newId);

    Q_EMIT nodeCreated(newId);

    return newId;
}

bool DynamicPortsModel::connectionPossible(ConnectionId const connectionId) const
{
    return !connectionExists(connectionId);
}

void DynamicPortsModel::addConnection(ConnectionId const connectionId)
{
    _connectivity.insert(connectionId);

    Q_EMIT connectionCreated(connectionId);
}

bool DynamicPortsModel::nodeExists(NodeId const nodeId) const
{
    return (_nodeIds.find(nodeId) != _nodeIds.end());
}

PortAddRemoveWidget *DynamicPortsModel::widget(NodeId nodeId) const
{
    auto it = _nodeWidgets.find(nodeId);
    if (it == _nodeWidgets.end()) {
        _nodeWidgets[nodeId] = new PortAddRemoveWidget(0,
                                                       0,
                                                       nodeId,
                                                       *const_cast<DynamicPortsModel *>(this));
    }

    return _nodeWidgets[nodeId];
}

QVariant DynamicPortsModel::nodeData(NodeId nodeId, NodeRole role) const
{
    Q_UNUSED(nodeId);

    QVariant result;

    switch (role) {
    case NodeRole::Type:
        result = QString("Default Node Type");
        break;

    case NodeRole::Position:
        result = _nodeGeometryData[nodeId].pos;
        break;

    case NodeRole::Size:
        result = _nodeGeometryData[nodeId].size;
        break;

    case NodeRole::CaptionVisible:
        result = true;
        break;

    case NodeRole::Caption:
        result = QString("Node");
        break;

    case NodeRole::Style: {
        auto style = StyleCollection::nodeStyle();
        result = style.toJson().toVariantMap();
    } break;

    case NodeRole::InternalData: {
        // Retrieve the stored image data (QPixmap) for the node.
        auto it = _nodeInternalData.find(nodeId);
        if (it != _nodeInternalData.end()) {
            result = it->second; // Return the stored QVariant
        }
    } break;

    case NodeRole::InPortCount:
        result = _nodePortCounts[nodeId].in;
        break;

    case NodeRole::OutPortCount:
        result = _nodePortCounts[nodeId].out;
        break;

    case NodeRole::Widget: {
        result = QVariant::fromValue(widget(nodeId));
        break;
    }
    }

    return result;
}

bool DynamicPortsModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value)
{
    bool result = false;

    switch (role) {
    case NodeRole::Type:
        break;
    case NodeRole::Position: {
        _nodeGeometryData[nodeId].pos = value.value<QPointF>();

        Q_EMIT nodePositionUpdated(nodeId);

        result = true;
    } break;

    case NodeRole::Size: {
        _nodeGeometryData[nodeId].size = value.value<QSize>();
        result = true;
    } break;

    case NodeRole::CaptionVisible:
        break;

    case NodeRole::Caption:
        break;

    case NodeRole::Style:
        break;

     case NodeRole::InternalData:
        // Store the image data (QPixmap) for the node.
        _nodeInternalData[nodeId] = value;
        result = true;
        break;

    case NodeRole::InPortCount:
        _nodePortCounts[nodeId].in = value.toUInt();
        widget(nodeId)->populateButtons(PortType::In, value.toUInt());
        break;

    case NodeRole::OutPortCount:
        _nodePortCounts[nodeId].out = value.toUInt();
        widget(nodeId)->populateButtons(PortType::Out, value.toUInt());
        break;

    case NodeRole::Widget:
        break;
    }

    return result;
}

QVariant DynamicPortsModel::portData(NodeId nodeId,
                                     PortType portType,
                                     PortIndex portIndex,
                                     PortRole role) const
{
    switch (role) {
    case PortRole::Data:
        return QVariant();
        break;

    case PortRole::DataType:
        return QVariant();
        break;

    case PortRole::ConnectionPolicyRole:
        return QVariant::fromValue(ConnectionPolicy::One);
        break;

    case PortRole::CaptionVisible:
        return true;
        break;

    case PortRole::Caption:
        if (portType == PortType::In)
            return QString::fromUtf8("Port In");
        else
            return QString::fromUtf8("Port Out");

        break;
    }

    return QVariant();
}

bool DynamicPortsModel::setPortData(
    NodeId nodeId, PortType portType, PortIndex portIndex, QVariant const &value, PortRole role)
{
    Q_UNUSED(nodeId);
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    Q_UNUSED(value);
    Q_UNUSED(role);

    return false;
}

bool DynamicPortsModel::deleteConnection(ConnectionId const connectionId)
{
    bool disconnected = false;

    auto it = _connectivity.find(connectionId);

    if (it != _connectivity.end()) {
        disconnected = true;

        _connectivity.erase(it);
    };

    if (disconnected)
        Q_EMIT connectionDeleted(connectionId);

    return disconnected;
}

bool DynamicPortsModel::deleteNode(NodeId const nodeId)
{
    // Delete connections to this node first.
    auto connectionIds = allConnectionIds(nodeId);
    for (auto &cId : connectionIds) {
        deleteConnection(cId);
    }

    _nodeIds.erase(nodeId);
    _nodeGeometryData.erase(nodeId);
    _nodePortCounts.erase(nodeId);
    _nodeWidgets.erase(nodeId);

    Q_EMIT nodeDeleted(nodeId);

    return true;
}

// QJsonObject DynamicPortsModel::saveNode(NodeId const nodeId) const
// {
//     QJsonObject nodeJson;

//     nodeJson["id"] = static_cast<qint64>(nodeId);

//     {
//         QPointF const pos = nodeData(nodeId, NodeRole::Position).value<QPointF>();

//         QJsonObject posJson;
//         posJson["x"] = pos.x();
//         posJson["y"] = pos.y();
//         nodeJson["position"] = posJson;

//         nodeJson["inPortCount"] = QString::number(_nodePortCounts[nodeId].in);
//         nodeJson["outPortCount"] = QString::number(_nodePortCounts[nodeId].out);
//     }
//     QVariant internalData = nodeData(nodeId, NodeRole::InternalData);
//     if (internalData.canConvert<QPixmap>()) {
//         QPixmap pixmap = internalData.value<QPixmap>();
//         QByteArray byteArray;
//         QBuffer buffer(&byteArray);
//         buffer.open(QIODevice::WriteOnly);
//         pixmap.save(&buffer, "PNG");
//         nodeJson["image-data"] = QString(buffer.data().toBase64());
//     }

//     return nodeJson;
// }
QJsonObject DynamicPortsModel::saveNode(NodeId const nodeId) const 
{
    QJsonObject nodeJson = AbstractGraphModel::saveNode(nodeId); // Call base implementation
    // Add custom serialization logic here
 
    // QJsonObject nodeJson;
    nodeJson["id"] = static_cast<qint64>(nodeId);

    // Save position
    QPointF const pos = nodeData(nodeId, NodeRole::Position).value<QPointF>();
    QJsonObject posJson;
    posJson["x"] = pos.x();
    posJson["y"] = pos.y();
    nodeJson["position"] = posJson;

    // Save port counts
    nodeJson["inPortCount"] = QString::number(_nodePortCounts[nodeId].in);
    nodeJson["outPortCount"] = QString::number(_nodePortCounts[nodeId].out);
    

      // Save image data as base64
    auto imageData = nodeData(nodeId, NodeRole::InternalData);
    if (!imageData.isNull()) {
        nodeJson["image-data"] = imageData.toJsonObject()["image-data"];
    }

    return nodeJson;
}

QJsonObject DynamicPortsModel::save() const
{
    QJsonObject sceneJson;

    QJsonArray nodesJsonArray;
    for (auto const nodeId : allNodeIds()) {
        nodesJsonArray.append(saveNode(nodeId));
    }
    sceneJson["nodes"] = nodesJsonArray;

    QJsonArray connJsonArray;
    for (auto const &cid : _connectivity) {
        connJsonArray.append(QtNodes::toJson(cid));
    }
    sceneJson["connections"] = connJsonArray;

    return sceneJson;
}

// void DynamicPortsModel::loadNode(QJsonObject const &nodeJson)
// {
//     NodeId restoredNodeId = static_cast<NodeId>(nodeJson["id"].toInt());

//     _nextNodeId = std::max(_nextNodeId, restoredNodeId + 1);

//     // Create new node.
//     _nodeIds.insert(restoredNodeId);

//     setNodeData(restoredNodeId, NodeRole::InPortCount, nodeJson["inPortCount"].toString().toUInt());

//     setNodeData(restoredNodeId,
//                 NodeRole::OutPortCount,
//                 nodeJson["outPortCount"].toString().toUInt());

//     {
//         QJsonObject posJson = nodeJson["position"].toObject();
//         QPointF const pos(posJson["x"].toDouble(), posJson["y"].toDouble());

//         setNodeData(restoredNodeId, NodeRole::Position, pos);
//     }
//         if (nodeJson.contains("image-data")) {
//         QByteArray imageData = QByteArray::fromBase64(nodeJson["image-data"].toString().toUtf8());
//         QPixmap pixmap;
//         pixmap.loadFromData(imageData);
//         setNodeData(restoredNodeId, NodeRole::InternalData, QVariant::fromValue(pixmap));
//     }

//     Q_EMIT nodeCreated(restoredNodeId);
// }

// void DynamicPortsModel::loadNode(QJsonObject const &nodeJson) 
// {
//     AbstractGraphModel::loadNode(nodeJson);
//     NodeId restoredNodeId = static_cast<NodeId>(nodeJson["id"].toInt());
//     _nextNodeId = std::max(_nextNodeId, restoredNodeId + 1);

//     // Restore position
//     QJsonObject posJson = nodeJson["position"].toObject();
//     QPointF const pos(posJson["x"].toDouble(), posJson["y"].toDouble());
//     setNodeData(restoredNodeId, NodeRole::Position, pos);

//     // Restore port counts
//     setNodeData(restoredNodeId, NodeRole::InPortCount, nodeJson["inPortCount"].toString().toUInt());
//     setNodeData(restoredNodeId,
//                 NodeRole::OutPortCount,
//                 nodeJson["outPortCount"].toString().toUInt());

//     // Restore internal data (if any)
//     if (nodeJson.contains("image-data")) {
//         QByteArray imageData = QByteArray::fromBase64(nodeJson["image-data"].toString().toUtf8());
//         QPixmap pixmap;
//         pixmap.loadFromData(imageData);
//         setNodeData(restoredNodeId, NodeRole::InternalData, QVariant::fromValue(pixmap));
//     }

//     Q_EMIT nodeCreated(restoredNodeId);
// }
void DynamicPortsModel::loadNode(QJsonObject const &nodeJson)
{
    AbstractGraphModel::loadNode(nodeJson);

    NodeId restoredNodeId = static_cast<NodeId>(nodeJson["id"].toInt());

    // Load image data from base64
    if (nodeJson.contains("image-data")) {
        QByteArray imageData = QByteArray::fromBase64(nodeJson["image-data"].toString().toUtf8());
        QImage qimg;
        qimg.loadFromData(imageData);
        cv::Mat image = QImageToCvMat(qimg); // Helper function to convert QImage to cv::Mat
        _nodeImages[restoredNodeId] = image;
    }
}
void DynamicPortsModel::load(QJsonObject const &jsonDocument)
{
    QJsonArray nodesJsonArray = jsonDocument["nodes"].toArray();

    for (QJsonValueRef nodeJson : nodesJsonArray) {
        loadNode(nodeJson.toObject());
    }

    QJsonArray connectionJsonArray = jsonDocument["connections"].toArray();

    for (QJsonValueRef connection : connectionJsonArray) {
        QJsonObject connJson = connection.toObject();

        ConnectionId connId = QtNodes::fromJson(connJson);

        // Restore the connection
        addConnection(connId);
    }
}

void DynamicPortsModel::addPort(NodeId nodeId, PortType portType, PortIndex portIndex)
{
    // STAGE 1.
    // Compute new addresses for the existing connections that are shifted and
    // placed after the new ones
    PortIndex first = portIndex;
    PortIndex last = first;
    portsAboutToBeInserted(nodeId, portType, first, last);

    // STAGE 2. Change the number of connections in your model
    if (portType == PortType::In)
        _nodePortCounts[nodeId].in++;
    else
        _nodePortCounts[nodeId].out++;

    // STAGE 3. Re-create previouly existed and now shifted connections
    portsInserted();

    Q_EMIT nodeUpdated(nodeId);
}

void DynamicPortsModel::removePort(NodeId nodeId, PortType portType, PortIndex portIndex)
{
    // STAGE 1.
    // Compute new addresses for the existing connections that are shifted upwards
    // instead of the deleted ports.
    PortIndex first = portIndex;
    PortIndex last = first;
    portsAboutToBeDeleted(nodeId, portType, first, last);

    // STAGE 2. Change the number of connections in your model
    if (portType == PortType::In)
        _nodePortCounts[nodeId].in--;
    else
        _nodePortCounts[nodeId].out--;

    portsDeleted();

    Q_EMIT nodeUpdated(nodeId);
}

void DynamicPortsModel::loadImage(NodeId nodeId)
{
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        tr("Open Image"),
        QString(),
        tr("Image Files (*.png *.jpg *.bmp *.tif *.tiff)"));
    if (fileName.isEmpty())
        return;

    cv::Mat image = cv::imread(fileName.toStdString());
    if (image.empty()) {
        qDebug() << "Failed to load image:" << fileName;
        return;
    }

    // Convert the image to base64 for storage
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    QImage qimg = cvMatToQImage(image);
    qimg.save(&buffer, "PNG");

    // Store the image data in the model
    QJsonObject imageDataJson;
    imageDataJson["image-data"] = QString(byteArray.toBase64());
    QVariant imageData = QVariant::fromValue(imageDataJson);

    setNodeData(nodeId, NodeRole::InternalData, imageData);
    Q_EMIT nodeUpdated(nodeId);
}
#include "AbstractNodeGeometry.hpp"

//Includes the graph model and the style definitions (like node/connection visual settings).
#include "AbstractGraphModel.hpp"
#include "StyleCollection.hpp"

//Includes Qt's QMargins class, which adds padding/margins around rectangles.
#include <QMargins>

#include <cmath>

namespace QtNodes {

AbstractNodeGeometry::AbstractNodeGeometry(AbstractGraphModel &graphModel)
    : _graphModel(graphModel)
{
    //
}

// Returns the outer bounding rectangle for a node, which may include visual padding/margins.
QRectF AbstractNodeGeometry::boundingRect(NodeId const nodeId) const
{
    QSize s = size(nodeId);

    double ratio = 0.20;

//Computes the margins based on node size.
    int widthMargin = s.width() * ratio;
    int heightMargin = s.height() * ratio;

//Creates a QMargins object with symmetric margins on all sides.
    QMargins margins(widthMargin, heightMargin, widthMargin, heightMargin);


// Creates a rectangle from (0,0) to the node’s size.
    QRectF r(QPointF(0, 0), s);

//Returns the rectangle with the margins added. This is the full area the node may occupy visually.
    return r.marginsAdded(margins);
}
//Returns the position of a port in scene coordinates (after applying transformation t)
QPointF AbstractNodeGeometry::portScenePosition(NodeId const nodeId,
                                                PortType const portType,
                                                PortIndex const index,
                                                QTransform const &t) const
{
    QPointF result = portPosition(nodeId, portType, index);

//Transforms the local point to the scene (global) coordinate system and returns it.
    return t.map(result);
}
//Given a point on the node, checks if it is "close enough" to any of the ports and returns that port's index if hit.
PortIndex AbstractNodeGeometry::checkPortHit(NodeId const nodeId,
                                             PortType const portType,
                                             QPointF const nodePoint) const
{
    
    //Gets the current node style, which defines visual properties like port diameter.
    auto const &nodeStyle = StyleCollection::nodeStyle();

    PortIndex result = InvalidPortIndex;

    if (portType == PortType::None)
        return result;
//Defines how close the click must be to count as a "hit" (2x port diameter).
    double const tolerance = 2.0 * nodeStyle.ConnectionPointDiameter;
//Gets the number of ports for the given port type (In/Out).
    size_t const n = _graphModel.nodeData<unsigned int>(nodeId,
                                                        (portType == PortType::Out)
                                                            ? NodeRole::OutPortCount
                                                            : NodeRole::InPortCount);

   //Loops over each port of the node.
    for (unsigned int portIndex = 0; portIndex < n; ++portIndex) {
        auto pp = portPosition(nodeId, portType, portIndex);

// Computes the vector from the test point to the port position.
        QPointF p = pp - nodePoint;
        
//Computes the Euclidean distance using the dot product.
        auto distance = std::sqrt(QPointF::dotProduct(p, p));

        if (distance < tolerance) {
            result = portIndex;
            break;
        }
    }

    return result;
}
QVariant AbstractGraphModel::nodeData(NodeId nodeId, NodeRole role) const
{
    QVariant result;
    switch (role) {
        // ... existing cases ...
        case NodeRole::InternalData: {
            auto it = _nodeInternalData.find(nodeId);
            if (it != _nodeInternalData.end()) {
                return it->second;  // Return QImage or QPixmap stored for the node
            }
            break;
        }
        default:
            qWarning() << "Unhandled NodeRole:" << static_cast<int>(role);
             break;
        // ... other cases ...
    }
    return result;
}
// QVariant AbstractGraphModel::nodeData(NodeId nodeId, NodeRole role) const
// {
//     QVariant result;
//     switch (role) {
//     case NodeRole::Type:
//         result = QString("Default Node Type");
//         break;
//     case NodeRole::Position:
//         result = _nodeGeometryData[nodeId].pos;
//         break;
//     case NodeRole::Size:
//         result = _nodeGeometryData[nodeId].size;
//         break;
//     case NodeRole::CaptionVisible:
//         result = true; // Default visibility
//         break;
//     case NodeRole::Caption:
//         result = QString("Node");
//         break;
//     case NodeRole::Style: {
//         auto style = StyleCollection::nodeStyle();
//         result = style.toJson().toVariantMap();
//         break;
//     }
//     case NodeRole::InternalData:
//         // Handle internal data (e.g., QImage)
//         if (_nodeInternalData.find(nodeId) != _nodeInternalData.end()) {
//             result = _nodeInternalData.at(nodeId);
//         }
//         break;
//     case NodeRole::InPortCount:
//         result = _nodePortCounts[nodeId].in;
//         break;
//     case NodeRole::OutPortCount:
//         result = _nodePortCounts[nodeId].out;
//         break;
//     case NodeRole::Widget: {
//         auto w = widget(nodeId);
//         result = QVariant::fromValue(w);
//         break;
//     }
//     default:
//         // Handle unknown roles (optional)
//         qWarning() << "Unhandled NodeRole:" << static_cast<int>(role);
//         break;
//     }
//     return result;
// }
bool AbstractGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value)
{
    bool result = false;
    switch (role) {
        // ... existing cases ...
        case NodeRole::InternalData: {
            _nodeInternalData[nodeId] = value;  // Store QImage or QPixmap
            result = true;
            break;
        }
      default:
          qWarning() << "Unhandled NodeRole:" << static_cast<int>(role);
          break;
    }
    return result;
}

QJsonObject AbstractGraphModel::saveNode(NodeId const nodeId) const
{
    QJsonObject nodeJson;
    nodeJson["id"] = static_cast<qint64>(nodeId);

    // Save position
    QPointF const pos = nodeData(nodeId, NodeRole::Position).value<QPointF>();
    QJsonObject posJson;
    posJson["x"] = pos.x();
    posJson["y"] = pos.y();
    nodeJson["position"] = posJson;

    // Save internal data (image)
    auto it = _nodeInternalData.find(nodeId);
    if (it != _nodeInternalData.end() && it->second.canConvert<QImage>()) {
        QImage image = it->second.value<QImage>();
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");  // Save image as PNG
        nodeJson["image-data"] = QString(buffer.data().toBase64());  // Base64 encode
    }

    return nodeJson;
}

void AbstractGraphModel::loadNode(QJsonObject const &nodeJson)
{
    NodeId restoredNodeId = static_cast<NodeId>(nodeJson["id"].toInt());
    _nextNodeId = std::max(_nextNodeId, restoredNodeId + 1);

    // Restore position
    QJsonObject posJson = nodeJson["position"].toObject();
    QPointF const pos(posJson["x"].toDouble(), posJson["y"].toDouble());
    setNodeData(restoredNodeId, NodeRole::Position, pos);

    // Restore internal data (image)
    if (nodeJson.contains("image-data")) {
        QByteArray imageData = QByteArray::fromBase64(nodeJson["image-data"].toString().toUtf8());
        QImage image;
        image.loadFromData(imageData);  // Load image from byte array
        setNodeData(restoredNodeId, NodeRole::InternalData, QVariant::fromValue(image));
    }

    Q_EMIT nodeCreated(restoredNodeId);
}
} // namespace QtNodes

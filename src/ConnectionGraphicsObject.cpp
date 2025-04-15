#include "ConnectionGraphicsObject.hpp"

#include "AbstractConnectionPainter.hpp"
#include "AbstractGraphModel.hpp"
#include "AbstractNodeGeometry.hpp"
#include "BasicGraphicsScene.hpp"
#include "ConnectionIdUtils.hpp"
#include "ConnectionState.hpp"
#include "ConnectionStyle.hpp"
#include "NodeConnectionInteraction.hpp"
#include "NodeGraphicsObject.hpp"
#include "StyleCollection.hpp"
#include "locateNode.hpp"

#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include <QtCore/QDebug>

#include <stdexcept>

namespace QtNodes {
//Creates a new visual connection object. Takes a reference to the scene and the connection ID.
ConnectionGraphicsObject::ConnectionGraphicsObject(BasicGraphicsScene &scene,
                                                   ConnectionId const connectionId)
    : _connectionId(connectionId)//stores which two nodes this connects.
    , _graphModel(scene.graphModel())//provides access to underlying data.
    , _connectionState(*this)//tracks hover/selection/etc
    , _out{0, 0}
    , _in{0, 0}
    //are coordinates for the ends of the wire.
{
    //Adds this QGraphicsItem (the connection) to the scene.
    scene.addItem(this);

  //Enables interaction: movement, focus, and selection.

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

   
    setAcceptHoverEvents(true);

    //addGraphicsEffect();
//Enables hover events; z-value puts it behind other items.

    setZValue(-1.0);
//Initializes position of the wire based on connected nodes.
    initializePosition();
}
//Sets the position of the wire when it's first created.


void ConnectionGraphicsObject::initializePosition()
{
    // This function is only called when the ConnectionGraphicsObject
    // is newly created. At this moment both end coordinates are (0, 0)
    // in Connection G.O. coordinates. The position of the whole
    // Connection G. O. in scene coordinate system is also (0, 0).
    // By moving the whole object to the Node Port position
    // we position both connection ends correctly.

//If this wire is partially connected (e.g., dragging to connect)
    if (_connectionState.requiredPort() != PortType::None) {
        PortType attachedPort = oppositePort(_connectionState.requiredPort());
//Get the already attached side (out/in)
//Identify which node and port is connected
        PortIndex portIndex = getPortIndex(attachedPort, _connectionId);
        NodeId nodeId = getNodeId(attachedPort, _connectionId);


//Get the visual object for the node
        NodeGraphicsObject *ngo = nodeScene()->nodeGraphicsObject(nodeId);

// If the node exists, calculate the scene position of the port and move the wire to that position.
        if (ngo) {
            QTransform nodeSceneTransform = ngo->sceneTransform();

            AbstractNodeGeometry &geometry = nodeScene()->nodeGeometry();

            QPointF pos = geometry.portScenePosition(nodeId,
                                                     attachedPort,
                                                     portIndex,
                                                     nodeSceneTransform);

            this->setPos(pos);
        }
    }

    move();
}
//Returns reference to the graph model.
AbstractGraphModel &ConnectionGraphicsObject::graphModel() const
{
    return _graphModel;
}
//Returns pointer to the parent scene cast as BasicGraphicsScene
BasicGraphicsScene *ConnectionGraphicsObject::nodeScene() const
{
    return dynamic_cast<BasicGraphicsScene *>(scene());
}
//Returns this object's connection ID.
ConnectionId const &ConnectionGraphicsObject::connectionId() const
{
    return _connectionId;
}
//Calculates the bounding rectangle for the connection line and its control points (for Bezier curves). Ensures enough room for the wire and end circles.
QRectF ConnectionGraphicsObject::boundingRect() const
{
    auto points = pointsC1C2();

    // `normalized()` fixes inverted rects.
    QRectF basicRect = QRectF(_out, _in).normalized();

    QRectF c1c2Rect = QRectF(points.first, points.second).normalized();

    QRectF commonRect = basicRect.united(c1c2Rect);

    auto const &connectionStyle = StyleCollection::connectionStyle();
    float const diam = connectionStyle.pointDiameter();
    QPointF const cornerOffset(diam, diam);

    // Expand rect by port circle diameter
    commonRect.setTopLeft(commonRect.topLeft() - cornerOffset);
    commonRect.setBottomRight(commonRect.bottomRight() + 2 * cornerOffset);

    return commonRect;
}

QPainterPath ConnectionGraphicsObject::shape() const
{
#ifdef DEBUG_DRAWING

    //QPainterPath path;

    //path.addRect(boundingRect());
    //return path;

#else
    return nodeScene()->connectionPainter().getPainterStroke(*this);
#endif
}

QPointF const &ConnectionGraphicsObject::endPoint(PortType portType) const
{
    Q_ASSERT(portType != PortType::None); //  Q_ASSERT

    return (portType == PortType::Out ? _out : _in);
}

void ConnectionGraphicsObject::setEndPoint(PortType portType, QPointF const &point)
{
    if (portType == PortType::In)
        _in = point;
    else
        _out = point;
}
//Updates the connection endpoints (_in, _out) based on the current positions of connected nodes and their ports.
void ConnectionGraphicsObject::move()
{
    auto moveEnd = [this](ConnectionId cId, PortType portType) {
        NodeId nodeId = getNodeId(portType, cId);

        if (nodeId == InvalidNodeId)
            return;

        NodeGraphicsObject *ngo = nodeScene()->nodeGraphicsObject(nodeId);

        if (ngo) {
            AbstractNodeGeometry &geometry = nodeScene()->nodeGeometry();

            QPointF scenePos = geometry.portScenePosition(nodeId,
                                                          portType,
                                                          getPortIndex(portType, cId),
                                                          ngo->sceneTransform());

            QPointF connectionPos = sceneTransform().inverted().map(scenePos);

            setEndPoint(portType, connectionPos);
        }
    };

    moveEnd(_connectionId, PortType::Out);
    moveEnd(_connectionId, PortType::In);

    prepareGeometryChange();

    update();
}
//Accessors for the connection state (hovered, selected, dragging)
ConnectionState const &ConnectionGraphicsObject::connectionState() const
{
    return _connectionState;
}

ConnectionState &ConnectionGraphicsObject::connectionState()
{
    return _connectionState;
}
//Delegates to the scene's ConnectionPainter to draw the wire
void ConnectionGraphicsObject::paint(QPainter *painter,
                                     QStyleOptionGraphicsItem const *option,
                                     QWidget *)
{
    if (!scene())
        return;

    painter->setClipRect(option->exposedRect);

    nodeScene()->connectionPainter().paint(painter, *this);
}
// Basic press handling, forwarding to base class.

void ConnectionGraphicsObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}
//Handles dragging:

// Detects nearby nodes using locateNodeAt().

// Updates hovered state.

// Updates endpoint of wire.

// Triggers repaint.
void ConnectionGraphicsObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    prepareGeometryChange();

    auto view = static_cast<QGraphicsView *>(event->widget());
    auto ngo = locateNodeAt(event->scenePos(), *nodeScene(), view->transform());
    if (ngo) {
        ngo->reactToConnection(this);

        _connectionState.setLastHoveredNode(ngo->nodeId());
    } else {
        _connectionState.resetLastHoveredNode();
    }

    //-------------------

    auto requiredPort = _connectionState.requiredPort();

    if (requiredPort != PortType::None) {
        setEndPoint(requiredPort, event->pos());
    }

    //-------------------

    update();

    event->accept();
}
//Attempts to finalize a connection to a node after dragging ends. If unsuccessful, it deletes the temporary connection.

void ConnectionGraphicsObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

    ungrabMouse();
    event->accept();

    auto view = static_cast<QGraphicsView *>(event->widget());

    Q_ASSERT(view);

    auto ngo = locateNodeAt(event->scenePos(), *nodeScene(), view->transform());

    bool wasConnected = false;

    if (ngo) {
        NodeConnectionInteraction interaction(*ngo, *this, *nodeScene());

        wasConnected = interaction.tryConnect();
    }

    // If connection attempt was unsuccessful
    if (!wasConnected) {
        // Resulting unique_ptr is not used and automatically deleted.
        nodeScene()->resetDraftConnection();
    }
}

//Handles visual updates when hovering the wire.

void ConnectionGraphicsObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    _connectionState.setHovered(true);

    update();

    // Signal
    nodeScene()->connectionHovered(connectionId(), event->screenPos());

    event->accept();
}

void ConnectionGraphicsObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    _connectionState.setHovered(false);

    update();

    // Signal
    nodeScene()->connectionHoverLeft(connectionId());

    event->accept();
}
//Chooses between horizontal/vertical orientations to compute Bezier control points.

std::pair<QPointF, QPointF> ConnectionGraphicsObject::pointsC1C2() const
{
    // /Computes smooth curve offsets depending on node positions.
    switch (nodeScene()->orientation()) {
    case Qt::Horizontal:
        return pointsC1C2Horizontal();
        break;

    case Qt::Vertical:
        return pointsC1C2Vertical();
        break;
    }

    throw std::logic_error("Unreachable code after switch statement");
}

void ConnectionGraphicsObject::addGraphicsEffect()
{
    auto effect = new QGraphicsBlurEffect;

    effect->setBlurRadius(5);
    setGraphicsEffect(effect);

    //auto effect = new QGraphicsDropShadowEffect;
    //auto effect = new ConnectionBlurEffect(this);
    //effect->setOffset(4, 4);
    //effect->setColor(QColor(Qt::gray).darker(800));
}

std::pair<QPointF, QPointF> ConnectionGraphicsObject::pointsC1C2Horizontal() const
{
    double const defaultOffset = 200;

    double xDistance = _in.x() - _out.x();

    double horizontalOffset = qMin(defaultOffset, std::abs(xDistance));

    double verticalOffset = 0;

    double ratioX = 0.5;

    if (xDistance <= 0) {
        double yDistance = _in.y() - _out.y() + 20;

        double vector = yDistance < 0 ? -1.0 : 1.0;

        verticalOffset = qMin(defaultOffset, std::abs(yDistance)) * vector;

        ratioX = 1.0;
    }

    horizontalOffset *= ratioX;

    QPointF c1(_out.x() + horizontalOffset, _out.y() + verticalOffset);

    QPointF c2(_in.x() - horizontalOffset, _in.y() - verticalOffset);

    return std::make_pair(c1, c2);
}

std::pair<QPointF, QPointF> ConnectionGraphicsObject::pointsC1C2Vertical() const
{
    double const defaultOffset = 200;

    double yDistance = _in.y() - _out.y();

    double verticalOffset = qMin(defaultOffset, std::abs(yDistance));

    double horizontalOffset = 0;

    double ratioY = 0.5;

    if (yDistance <= 0) {
        double xDistance = _in.x() - _out.x() + 20;

        double vector = xDistance < 0 ? -1.0 : 1.0;

        horizontalOffset = qMin(defaultOffset, std::abs(xDistance)) * vector;

        ratioY = 1.0;
    }

    verticalOffset *= ratioY;

    QPointF c1(_out.x() + horizontalOffset, _out.y() + verticalOffset);

    QPointF c2(_in.x() - horizontalOffset, _in.y() - verticalOffset);

    return std::make_pair(c1, c2);
}

} // namespace QtNodes

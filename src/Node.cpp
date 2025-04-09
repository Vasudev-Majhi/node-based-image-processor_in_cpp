#include "Node.h"
#include "Port.h"
#include "Connection.h"
#include <QPainter>

Node::Node(QGraphicsItem* parent) 
    : QObject(), QGraphicsItem(parent), nodeColor(Qt::darkGray) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

Node::~Node() {
    for(Port* port : inputPorts) delete port;
    for(Port* port : outputPorts) delete port;
}

QRectF Node::boundingRect() const {
    return QRectF(-width/2, -height/2, width, height);
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setBrush(nodeColor);
    painter->drawRoundedRect(-width/2, -height/2, width, height, 5, 5);
    painter->setBrush(Qt::darkGray);
    painter->drawRoundedRect(-width/2, -height/2, width, 20, 5, 5);
    
    int y = -height/2 + 30;
    for(Port* port : inputPorts) {
        port->setPos(-width/2 - Port::DIAMETER/2, y);
        y += 20;
    }
    
    y = -height/2 + 30;
    for(Port* port : outputPorts) {
        port->setPos(width/2 + Port::DIAMETER/2, y);
        y += 20;
    }
}

void Node::addInputPort(const QString& name) {
    Port* port = new Port(this, Port::Input);
    inputPorts.push_back(port);
    if(scene()) scene()->addItem(port);
}

void Node::addOutputPort(const QString& name) {
    Port* port = new Port(this, Port::Output);
    outputPorts.push_back(port);
    if(scene()) scene()->addItem(port);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged) {
        for(Port* port : inputPorts) {
            for(Connection* conn : port->connections) {
                conn->updatePath();
            }
        }
        for(Port* port : outputPorts) {
            for(Connection* conn : port->connections) {
                conn->updatePath();
            }
        }
    }
    return QGraphicsItem::itemChange(change, value);
}
#include "Port.h"
#include "Node.h"
#include <QPainter>

Port::Port(Node* parent, PortType type) 
    : QGraphicsItem(parent), parentNode(parent), portType(type) {
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QRectF Port::boundingRect() const {
    return QRectF(-DIAMETER/2, -DIAMETER/2, DIAMETER, DIAMETER);
}

void Port::paint(QPainter* painter, 
                const QStyleOptionGraphicsItem* option,
                QWidget* widget) {
    painter->setBrush(portType == Input ? Qt::red : Qt::green);
    painter->drawEllipse(-DIAMETER/2, -DIAMETER/2, DIAMETER, DIAMETER);
}
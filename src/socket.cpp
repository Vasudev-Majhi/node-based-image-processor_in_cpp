#include "socket.h"
#include <QPainter>
#include "node.h"

Socket::Socket(SocketType type, QGraphicsItem* parent)
    : QGraphicsItem(parent), 
     m_type(type), 
     m_node(qgraphicsitem_cast<Node*>(parent))
     {
     setFlag(ItemSendsGeometryChanges);
     }

Socket::~Socket() {
}

SocketType Socket::socketType() const {
    return m_type;
}

Node* Socket::node() const {
    return m_node;
}

QRectF Socket::boundingRect() const {
    return QRectF(-5, -5, 10, 10);
}

void Socket::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (m_type == INPUT) {
        painter->setBrush(Qt::blue);
    } else {
        painter->setBrush(Qt::red);
    }
    painter->drawEllipse(boundingRect());
}
int Socket::type() const {
    return QGraphicsItem::UserType + 4;
}
#include "node.h"
#include <QPainter>
#include <QGraphicsScene>

Node::Node(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name), m_color(Qt::lightGray) {
}

Node::~Node() {
    qDeleteAll(m_sockets);
}

void Node::addSocket(Socket* socket) {
    m_sockets.append(socket);
}

QList<Socket*> Node::sockets() const {
    return m_sockets;
}

QRectF Node::boundingRect() const {
    return QRectF(-100, -30, 200, 60 + m_sockets.size() * 30);
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setBrush(m_color);
    painter->drawRoundedRect(boundingRect().adjusted(5, 5, -5, -5), 10, 10);

    painter->setPen(Qt::black);
    painter->drawText(QRectF(-80, -25, 160, 30), Qt::AlignCenter, m_name);

    for (Socket* socket : m_sockets) {
        QPointF pos = socket->pos();
        painter->drawEllipse(pos.x() - 5, pos.y() - 5, 10, 10);
    }
}
int Node::type() const {
    return QGraphicsItem::UserType + 3; 
}
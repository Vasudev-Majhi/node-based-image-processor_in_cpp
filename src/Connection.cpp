#include "connection.h"
#include "socket.h"
#include <QPainter>
#include <QGraphicsScene>

Connection::Connection(Socket* from, Socket* to, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_from(from), m_to(to) {
}

Connection::~Connection() {
}

Socket* Connection::fromSocket() const {
    return m_from;
}

Socket* Connection::toSocket() const {
    return m_to;
}

QRectF Connection::boundingRect() const {
    QPointF fromPos = m_from->pos() + m_from->parentItem()->pos();
    QPointF toPos = m_to->pos() + m_to->parentItem()->pos();
    QLineF line(fromPos, toPos);

    qreal extra = 1.0;
    return QRectF(line.p1(), QSizeF(line.p2().x() - line.p1().x(), line.p2().y() - line.p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
    
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QPointF fromPos = m_from->pos() + m_from->parentItem()->pos();
    QPointF toPos = m_to->pos() + m_to->parentItem()->pos();
    QLineF line(fromPos, toPos);

    QPen pen(Qt::black, 2);
    painter->setPen(pen);

    painter->drawLine(line);
}
int Connection::type() const {
    return QGraphicsItem::UserType + 2; // 
}
#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QGraphicsItem>
#include <QLineF>
#include "socket.h"

// class Socket;
class Socket;
class Node;

class Connection : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Connection(Socket* from, Socket* to, QGraphicsItem* parent = nullptr);
    ~Connection();

    Socket* fromSocket() const;
    Socket* toSocket() const;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;

private:
    Socket* m_from;
    Socket* m_to;
};

#endif // CONNECTION_H
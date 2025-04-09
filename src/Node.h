#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QGraphicsItem>
#include <QList>
#include <QColor>
#include <QString>
#include <QPointF>
#include "socket.h"
#include <opencv2/opencv.hpp>


class Node : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Node(const QString& name, QGraphicsItem* parent = nullptr);
    ~Node();

    void addSocket(Socket* socket);
    QList<Socket*> sockets() const;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    virtual void process() = 0;
    int type() const override;

protected:
    QString m_name;
    QList<Socket*> m_sockets;
    QColor m_color;
};

#endif // NODE_H
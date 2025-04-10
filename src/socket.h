#ifndef SOCKET_H
#define SOCKET_H



#include <QGraphicsItem>

class Node;

enum SocketType {
    INPUT,
    OUTPUT
};

class Socket : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Socket(SocketType type, QGraphicsItem* parent = nullptr);
    ~Socket();

   SocketType socketType() const; 
    Node* node() const;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;

    void setSocketData(const QVariant& d) 
    { m_data = d; }

    QVariant socketData() const
    { return m_data; }

private:
    QVariant m_data;
    SocketType m_type;
    Node* m_node;
};

#endif // SOCKET_H
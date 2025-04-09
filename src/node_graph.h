#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H


#include <QObject>
#include <QList>
#include <QGraphicsScene>
#include "node.h"
#include "connection.h"

// class Node;
// class Connection;

class NodeGraph : public QObject {
    Q_OBJECT
public:
    NodeGraph(QObject* parent = nullptr);
    ~NodeGraph();

    void addNode(Node* node);
    void addConnection(Connection* connection);
    void process();

private:
    QList<Node*> m_nodes;
    QList<Connection*> m_connections;

    QList<Node*> topologicalSort();
};

#endif // NODE_GRAPH_H
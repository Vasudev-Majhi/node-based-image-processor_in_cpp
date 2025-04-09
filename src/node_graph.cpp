#include "node_graph.h"
#include "node.h"
#include "connection.h"
#include "socket.h"
#include <QMap>
#include <algorithm>
#include <queue>

NodeGraph::NodeGraph(QObject* parent)
    : QObject(parent) {
}

NodeGraph::~NodeGraph() {
    qDeleteAll(m_connections);
    qDeleteAll(m_nodes);
}

void NodeGraph::addNode(Node* node) {
    m_nodes.append(node);
}

void NodeGraph::addConnection(Connection* connection) {
    m_connections.append(connection);
}

QList<Node*> NodeGraph::topologicalSort() {
    QList<Node*> sorted;
    std::queue<Node*> queue;

    // Initialize in-degree for each node
    QMap<Node*, int> inDegree;
    QMap<Node*, QList<Connection*>> adjacency;

    for (Node* node : m_nodes) {
        inDegree[node] = 0;
    }

    for (Connection* connection : m_connections) {
        Node* from = connection->fromSocket()->node();
        Node* to = connection->toSocket()->node();

        adjacency[from].append(connection);
        inDegree[to]++;
    }

    // Add nodes with in-degree 0 to the queue
    for (auto it = inDegree.begin(); it != inDegree.end(); ++it) {
        if (it.value() == 0) {
            queue.push(it.key());
        }
    }

    // Process nodes
    while (!queue.empty()) {
        Node* node = queue.front();
        queue.pop();
        sorted.append(node);

        for (Connection* connection : adjacency[node]) {
            Node* to = connection->toSocket()->node();
            inDegree[to]--;

            if (inDegree[to] == 0) {
                queue.push(to);
            }
        }
    }

    return sorted;
}

void NodeGraph::process() {
    QList<Node*> sorted = topologicalSort();
    for (Node* node : sorted) {
        node->process();
    }
}
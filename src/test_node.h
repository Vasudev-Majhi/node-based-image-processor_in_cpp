// test_node.h
#ifndef TEST_NODE_H
#define TEST_NODE_H

#include "node.h"

class TestNode : public Node {
    Q_OBJECT
public:
    TestNode(const QString& name, QGraphicsItem* parent = nullptr);
    void process() override;
};

#endif // TEST_NODE_H
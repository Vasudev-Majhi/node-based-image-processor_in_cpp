#ifndef IMAGE_INPUT_NODE_H
#define IMAGE_INPUT_NODE_H


#include "node.h"
#include <opencv2/opencv.hpp>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <QMetaType>
// Register cv::Mat with Qt meta-object system


class ImageInputNode : public Node {
    Q_OBJECT
public:
    explicit ImageInputNode(const QString& name = "Image Input", QGraphicsItem* parent = nullptr);
    void process() override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void loadImage();
    // Override the QGraphicsItem context menu event
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    cv::Mat m_image;
    QFileInfo m_fileInfo;
    Socket* m_outputSocket;

    QImage cvMatToQImage(const cv::Mat& mat) const;
};

#endif // IMAGE_INPUT_NODE_H

#include "ImageViewerNode.h"
#include <QPainter>

ImageViewerNode::ImageViewerNode(QGraphicsItem* parent) 
    : Node(parent) {
    this->width = 256;
    this->height = 256;
    addInputPort("Image Input");
}

cv::Mat ImageViewerNode::processData(cv::Mat input) {
    if(!input.empty()) {
        currentPixmap = QPixmap::fromImage(cvMatToQImage(input));
        update();
    }
    return cv::Mat();
}

void ImageViewerNode::paint(QPainter* painter, 
                           const QStyleOptionGraphicsItem* option,
                           QWidget* widget) {
    Node::paint(painter, option, widget);
    
    if(!currentPixmap.isNull()) {
        QRectF baseRect = boundingRect();
        painter->drawPixmap(
            baseRect.x() + 5,
            baseRect.y() + 25,
            baseRect.width() - 10,
            baseRect.height() - 40,
            currentPixmap
        );
    }
}

QImage ImageViewerNode::cvMatToQImage(const cv::Mat& mat) {
    if(mat.type() == CV_8UC3) {
        return QImage(mat.data, mat.cols, mat.rows, 
                     mat.step, QImage::Format_RGB888).rgbSwapped();
    }
    return QImage();
}
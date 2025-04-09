#pragma once
#include "Node.h"
#include <QImage>
#include <QPixmap>

class ImageViewerNode : public Node {
    Q_OBJECT
public:
    ImageViewerNode(QGraphicsItem* parent = nullptr);
    cv::Mat processData(cv::Mat input) override;
    
protected:
    void paint(QPainter* painter, 
              const QStyleOptionGraphicsItem* option,
              QWidget* widget) override;

private:
    QPixmap currentPixmap;
    QImage cvMatToQImage(const cv::Mat& mat);
};
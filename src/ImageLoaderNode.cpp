#include "ImageLoaderNode.h"
#include <opencv2/opencv.hpp>

ImageLoaderNode::ImageLoaderNode(QGraphicsItem* parent) 
    : Node(parent) {
    addOutputPort("Image");
    loadedImage = cv::imread("image.png");
}

cv::Mat ImageLoaderNode::processData(cv::Mat) {
    return loadedImage.clone();
}
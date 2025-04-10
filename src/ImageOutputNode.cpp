#include "ImageOutputNode.h"
#include <QFileDialog>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QDebug>
#include <QPainter>
#include <QMetaType>
Q_DECLARE_METATYPE(cv::Mat)
OutputNode::OutputNode(const QString& name, QGraphicsItem* parent)
    : Node(name, parent)
{
    // Create and position the input socket
    m_inputSocket = new Socket(SocketType::INPUT, this);
    m_inputSocket->setPos(-90, 0);
    addSocket(m_inputSocket);

    // Allow selection and context menu
    setAcceptDrops(true);
    setFlags(ItemIsMovable | ItemIsSelectable);
}

void OutputNode::process()
{
    // Pull image from socket
     m_currentImage = m_inputSocket->socketData().value<cv::Mat>();
    if (m_currentImage.empty()) {
        qDebug() << "No image to output";
        return;
    }

    // Update the preview thumbnail
    updatePreview();
}

void OutputNode::saveImage()
{
    if (m_currentImage.empty()) return;

    // Prompt for save path
    QString filter = tr("PNG Files (*.png);;JPEG Files (*.jpg);;BMP Files (*.bmp)");
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save Image"), QString(), filter);
    if (fileName.isEmpty()) return;

    // Determine format & params
    std::vector<int> params;
    if (fileName.endsWith(".jpg", Qt::CaseInsensitive)) {
        params = { cv::IMWRITE_JPEG_QUALITY, m_quality };
    }

    // Write via OpenCV
    if (cv::imwrite(fileName.toStdString(), m_currentImage, params)) {
        qDebug() << "Saved image to" << fileName;
    } else {
        qDebug() << "Failed to save image to" << fileName;
    }
}

void OutputNode::updatePreview()
{
    // Convert cv::Mat to QImage
    m_previewImage = cvMatToQImage(m_currentImage)
                        .scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // Trigger repaint
    update();
}

QImage OutputNode::cvMatToQImage(const cv::Mat& mat) const
{
    switch (mat.type()) {
    case CV_8UC3: {
        QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    case CV_8UC1: {
        QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return img.copy();
    }
    case CV_8UC4: {
        QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return img.copy();
    }
    default: {
        cv::Mat tmp;
        mat.convertTo(tmp, CV_8UC3);
        return cvMatToQImage(tmp);
    }
    }
}

void OutputNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Draw the base node
    Node::paint(painter, option, widget);

    // Draw the preview thumbnail if available
    if (!m_previewImage.isNull()) {


        QRectF bounds = boundingRect();
        QRect thumbRect(bounds.left() + 10, bounds.top() + 10,
                m_previewImage.width(), m_previewImage.height());
        
                        
        painter->drawImage(thumbRect, m_previewImage);
    }
}

void OutputNode::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* saveAction = menu.addAction(tr("Save Image"));
    connect(saveAction, &QAction::triggered, this, &OutputNode::saveImage);
    menu.exec(event->screenPos());
}

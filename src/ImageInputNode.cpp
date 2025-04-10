#include "ImageInputNode.h"
#include <QDebug>
#include <QGraphicsSceneContextMenuEvent>
// #include "MetaTypes.h
#include <QMetaType>

Q_DECLARE_METATYPE(cv::Mat)

ImageInputNode::ImageInputNode(const QString& name, QGraphicsItem* parent)
    : Node(name, parent)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    // Create and position output socket
    m_outputSocket = new Socket(SocketType::OUTPUT, this);
    m_outputSocket->setPos(90, 0);
    addSocket(m_outputSocket);

    // Enable drag/drop and selection
    setAcceptDrops(true);
    setFlags(ItemIsMovable | ItemIsSelectable);
}

void ImageInputNode::process()
{
    if (m_image.empty()) {
        qDebug() << "No image loaded";
        return;
    }
    // Propagate image through output socket
   m_outputSocket->setSocketData(QVariant::fromValue(m_image));
}

void ImageInputNode::loadImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        tr("Open Image"),
        QString(),
        tr("Image Files (*.png *.jpg *.bmp *.tif *.tiff)"));

    if (fileName.isEmpty())
        return;

    m_image = cv::imread(fileName.toStdString());
    if (m_image.empty()) {
        qDebug() << "Failed to load image:" << fileName;
        return;
    }

    m_fileInfo = QFileInfo(fileName);
    // Force a repaint to show preview & metadata
    update();
}

void ImageInputNode::paint(QPainter* painter,
                           const QStyleOptionGraphicsItem* option,
                           QWidget* widget)
{
    // Draw the base node


    if (!m_image.empty()) {
        // Convert and scale
        QImage qimg = cvMatToQImage(m_image);
        QImage thumb = qimg.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Get node size
        QRectF br = boundingRect();
        int w = static_cast<int>(br.width());
        int h = static_cast<int>(br.height());

        // Position thumbnail
        QRect thumbRect(
            -w/2 + 10,
            -h/2 + 10,
            thumb.width(),
            thumb.height()
        );
        painter->drawImage(thumbRect, thumb);

        // Draw metadata
        QString meta = QString("%1×%2\n%3 KB\n%4")
                           .arg(m_image.cols)
                           .arg(m_image.rows)
                           .arg(m_fileInfo.size() / 1024)
                           .arg(m_fileInfo.suffix().toUpper());

        QRect textRect = thumbRect.translated(thumb.width() + 10, 0);
        painter->setPen(Qt::black);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop, meta);
    }
}


QImage ImageInputNode::cvMatToQImage(const cv::Mat& mat) const
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
        default:
            // Convert other types to 8UC3 then recurse
            cv::Mat tmp;
            mat.convertTo(tmp, CV_8UC3);
            return cvMatToQImage(tmp);
    }
}

void ImageInputNode::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* loadAction = menu.addAction(tr("Load Image"));
    connect(loadAction, &QAction::triggered, this, &ImageInputNode::loadImage);
    menu.exec(event->screenPos());
}

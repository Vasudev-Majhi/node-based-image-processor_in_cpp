#include "BlendModel.hpp"
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>

BlendModel::BlendModel()
{
    _widget = new QWidget;
    _previewLabel = new QLabel("Blended Output");
    _blendModeBox = new QComboBox;

    _blendModeBox->addItems({ "Normal", "Multiply", "Screen", "Overlay", "Difference" });

    auto *layout = new QVBoxLayout(_widget);
    layout->addWidget(_previewLabel);
    layout->addWidget(_blendModeBox);

    connect(_blendModeBox, &QComboBox::currentTextChanged, this, &BlendModel::blend);
}

unsigned int BlendModel::nPorts(QtNodes::PortType portType) const
{
    return (portType == QtNodes::PortType::In) ? 2 : 1;
}

QtNodes::NodeDataType BlendModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const
{
    return PixmapData().type();
}

std::shared_ptr<QtNodes::NodeData> BlendModel::outData(QtNodes::PortIndex)
{
    return std::make_shared<PixmapData>(_outputPixmap);
}

void BlendModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex)
{
    if (auto d = std::dynamic_pointer_cast<PixmapData>(nodeData)) {
        if (portIndex == 0)
            _pixmap1 = d->pixmap();
        else
            _pixmap2 = d->pixmap();
    }

    blend();
    Q_EMIT dataUpdated(0);
}

cv::Mat BlendModel::blendImages(const cv::Mat &img1, const cv::Mat &img2, const QString &mode)
{
    cv::Mat result;
    cv::Mat a, b;
    cv::resize(img2, b, img1.size());
    a = img1;

    if (mode == "Normal") {
        result = b.clone();
    } else if (mode == "Multiply") {
        cv::multiply(a, b, result, 1.0 / 255.0);
    } else if (mode == "Screen") {
        result = 255 - ((255 - a).mul(255 - b) / 255);
    } else if (mode == "Overlay") {
        result = a.clone();
        for (int y = 0; y < a.rows; ++y) {
            for (int x = 0; x < a.cols; ++x) {
                for (int c = 0; c < 3; ++c) {
                    float A = a.at<cv::Vec3b>(y, x)[c] / 255.0f;
                    float B = b.at<cv::Vec3b>(y, x)[c] / 255.0f;
                    float O = (A < 0.5f) ? (2 * A * B) : (1 - 2 * (1 - A) * (1 - B));
                    result.at<cv::Vec3b>(y, x)[c] = static_cast<uchar>(O * 255);
                }
            }
        }
    } else if (mode == "Difference") {
        cv::absdiff(a, b, result);
    } else {
        result = b.clone(); // Fallback to normal
    }

    return result;
}

void BlendModel::blend()
{
    if (_pixmap1.isNull() || _pixmap2.isNull())
        return;

    QImage img1 = _pixmap1.toImage().convertToFormat(QImage::Format_RGB888);
    QImage img2 = _pixmap2.toImage().convertToFormat(QImage::Format_RGB888);

    cv::Mat cvImg1(img1.height(), img1.width(), CV_8UC3, const_cast<uchar *>(img1.bits()), img1.bytesPerLine());
    cv::Mat cvImg2(img2.height(), img2.width(), CV_8UC3, const_cast<uchar *>(img2.bits()), img2.bytesPerLine());

    QString blendMode = _blendModeBox->currentText();
    cv::Mat blended = blendImages(cvImg1, cvImg2, blendMode);

    QImage resultImg(blended.data, blended.cols, blended.rows, blended.step, QImage::Format_RGB888);
    _outputPixmap = QPixmap::fromImage(resultImg.rgbSwapped());

    _previewLabel->setPixmap(_outputPixmap.scaled(200, 200, Qt::KeepAspectRatio));
}

#include "EdgeDetectionModel.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

#include <QImage>
#include <QPixmap>

EdgeDetectionModel::EdgeDetectionModel()
    : _previewLabel(new QLabel("Edge Detected Image")), _output(nullptr)
{
    _previewLabel->setAlignment(Qt::AlignCenter);
    _previewLabel->setMinimumSize(200, 200);

    _methodCombo = new QComboBox();
    _methodCombo->addItems({ "Sobel", "Canny" });

    _threshold1Slider = new QSlider(Qt::Horizontal);
    _threshold1Slider->setRange(0, 255);
    _threshold1Slider->setValue(50);

    _threshold2Slider = new QSlider(Qt::Horizontal);
    _threshold2Slider->setRange(0, 255);
    _threshold2Slider->setValue(150);

    _kernelSizeSlider = new QSlider(Qt::Horizontal);
    _kernelSizeSlider->setRange(1, 7);
    _kernelSizeSlider->setValue(3);
    _kernelSizeSlider->setSingleStep(2);
    _kernelSizeSlider->setPageStep(2);

    _overlayCheckBox = new QCheckBox("Overlay on original");

    auto layout = new QVBoxLayout();
    layout->addWidget(_previewLabel);
    layout->addWidget(new QLabel("Method:"));
    layout->addWidget(_methodCombo);
    layout->addWidget(new QLabel("Threshold 1:"));
    layout->addWidget(_threshold1Slider);
    layout->addWidget(new QLabel("Threshold 2:"));
    layout->addWidget(_threshold2Slider);
    layout->addWidget(new QLabel("Kernel Size:"));
    layout->addWidget(_kernelSizeSlider);
    layout->addWidget(_overlayCheckBox);

    _widget = new QWidget();
    _widget->setLayout(layout);

    connect(_methodCombo, &QComboBox::currentTextChanged, this, &EdgeDetectionModel::processImage);
    connect(_threshold1Slider, &QSlider::valueChanged, this, &EdgeDetectionModel::processImage);
    connect(_threshold2Slider, &QSlider::valueChanged, this, &EdgeDetectionModel::processImage);
    connect(_kernelSizeSlider, &QSlider::valueChanged, this, &EdgeDetectionModel::processImage);
    connect(_overlayCheckBox, &QCheckBox::stateChanged, this, &EdgeDetectionModel::processImage);
}

unsigned int EdgeDetectionModel::nPorts(QtNodes::PortType portType) const {
    return (portType == QtNodes::PortType::In || portType == QtNodes::PortType::Out) ? 1 : 0;
}

QtNodes::NodeDataType EdgeDetectionModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const {
    return PixmapData().type();
}

std::shared_ptr<QtNodes::NodeData> EdgeDetectionModel::outData(QtNodes::PortIndex) {
    return _output;
}

void EdgeDetectionModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex) {
    auto d = std::dynamic_pointer_cast<PixmapData>(nodeData);
    if (d) {
        _originalPixmap = d->pixmap();
        _previewLabel->setToolTip(QString("Size: %1 x %2").arg(_originalPixmap.width()).arg(_originalPixmap.height()));
        processImage();
    }
}

void EdgeDetectionModel::processImage() {
    if (_originalPixmap.isNull()) return;

    QImage qImage = _originalPixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat input(qImage.height(), qImage.width(), CV_8UC3, const_cast<uchar*>(qImage.bits()), qImage.bytesPerLine());
    cv::Mat gray, edges;

    cv::cvtColor(input, gray, cv::COLOR_RGB2GRAY);

    int ksize = _kernelSizeSlider->value() | 1;
    QString method = _methodCombo->currentText();

    if (method == "Sobel") {
        cv::Mat grad_x, grad_y;
        cv::Sobel(gray, grad_x, CV_16S, 1, 0, ksize);
        cv::Sobel(gray, grad_y, CV_16S, 0, 1, ksize);
        cv::Mat abs_grad_x, abs_grad_y;
        cv::convertScaleAbs(grad_x, abs_grad_x);
        cv::convertScaleAbs(grad_y, abs_grad_y);
        cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, edges);
    } else {
        cv::Canny(gray, edges, _threshold1Slider->value(), _threshold2Slider->value(), ksize);
    }

    if (_overlayCheckBox->isChecked()) {
        cv::Mat colorEdges;
        cv::cvtColor(edges, colorEdges, cv::COLOR_GRAY2BGR);
        cv::addWeighted(input, 0.7, colorEdges, 0.3, 0, input);
    } else {
        cv::cvtColor(edges, input, cv::COLOR_GRAY2BGR);
    }

    QImage result(input.data, input.cols, input.rows, static_cast<int>(input.step), QImage::Format_RGB888);
    QPixmap resultPixmap = QPixmap::fromImage(result.rgbSwapped());

    _output = std::make_shared<PixmapData>(resultPixmap);
    updateDisplay(resultPixmap);
    Q_EMIT dataUpdated(0);
}

void EdgeDetectionModel::updateDisplay(const QPixmap& pixmap) {
    _previewLabel->setPixmap(pixmap.scaled(_previewLabel->size(), Qt::KeepAspectRatio));
}

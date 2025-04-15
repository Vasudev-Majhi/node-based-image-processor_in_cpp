#include "BrightnessContrastModel.hpp"
#include <QImage>
#include <QPainter>

BrightnessContrastModel::BrightnessContrastModel()
{
    _widget = new QWidget;
    auto *layout = new QVBoxLayout(_widget);

    _previewLabel = new QLabel("No image");
    _previewLabel->setAlignment(Qt::AlignCenter);
    _previewLabel->setMinimumSize(200, 200);

    _brightnessSlider = new QSlider(Qt::Horizontal);
    _brightnessSlider->setRange(-100, 100);
    _brightnessSlider->setValue(0);

    _contrastSlider = new QSlider(Qt::Horizontal);
    _contrastSlider->setRange(-100, 100);
    _contrastSlider->setValue(0);

    layout->addWidget(_previewLabel);
    layout->addWidget(new QLabel("Brightness"));
    layout->addWidget(_brightnessSlider);
    layout->addWidget(new QLabel("Contrast"));
    layout->addWidget(_contrastSlider);

    connect(_brightnessSlider, &QSlider::valueChanged, this, &BrightnessContrastModel::onValueChanged);
    connect(_contrastSlider, &QSlider::valueChanged, this, &BrightnessContrastModel::onValueChanged);
}

QString BrightnessContrastModel::caption() const {
    return QString("Brightness/Contrast");
}

QString BrightnessContrastModel::name() const {
    return QString("BrightnessContrastModel");
}

QString BrightnessContrastModel::modelName() const {
    return QString("Brightness/Contrast");
}

unsigned int BrightnessContrastModel::nPorts(QtNodes::PortType portType) const {
    return (portType == QtNodes::PortType::In || portType == QtNodes::PortType::Out) ? 1 : 0;
}

QtNodes::NodeDataType BrightnessContrastModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const {
    return PixmapData().type();
}

std::shared_ptr<QtNodes::NodeData> BrightnessContrastModel::outData(QtNodes::PortIndex) {
    return std::make_shared<PixmapData>(_processedPixmap);
}

void BrightnessContrastModel::setInData(std::shared_ptr<QtNodes::NodeData> data, QtNodes::PortIndex) {
    _nodeData = data;
    if (auto d = std::dynamic_pointer_cast<PixmapData>(data)) {
        _originalPixmap = d->pixmap();
        _previewLabel->setPixmap(_originalPixmap.scaled(_previewLabel->size(), Qt::KeepAspectRatio));
         _previewLabel->setToolTip(QString("Size: %1 x %2")
            .arg(_originalPixmap.width())
            .arg(_originalPixmap.height()));
        onValueChanged();
    }
}

QWidget *BrightnessContrastModel::embeddedWidget() {
    return _widget;
}

void BrightnessContrastModel::onValueChanged() {
    if (_originalPixmap.isNull()) return;

    QImage img = _originalPixmap.toImage().convertToFormat(QImage::Format_ARGB32);

    int brightness = _brightnessSlider->value();
    int contrast = _contrastSlider->value();

    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor clr = QColor::fromRgba(line[x]);
            int r = qBound(0, ((clr.red() - 127) * contrast / 100) + 127 + brightness, 255);
            int g = qBound(0, ((clr.green() - 127) * contrast / 100) + 127 + brightness, 255);
            int b = qBound(0, ((clr.blue() - 127) * contrast / 100) + 127 + brightness, 255);
            line[x] = qRgba(r, g, b, clr.alpha());
        }
    }

    _processedPixmap = QPixmap::fromImage(img);
    _previewLabel->setPixmap(_processedPixmap.scaled(_previewLabel->size(), Qt::KeepAspectRatio));
    Q_EMIT dataUpdated(0);
}

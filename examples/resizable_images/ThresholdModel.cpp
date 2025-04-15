#include "ThresholdModel.hpp"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtCore/QEvent>

ThresholdModel::ThresholdModel()
    : _label(new QLabel("Binary Image will appear here")),
      _slider(new QSlider(Qt::Horizontal)),
      _widget(new QWidget),
      _layout(new QVBoxLayout(_widget))
{
    _label->setAlignment(Qt::AlignCenter);
    _label->setMinimumSize(200, 200);
    _label->installEventFilter(this);

    _slider->setMinimum(0);
    _slider->setMaximum(255);
    _slider->setValue(_thresholdValue);
    _slider->setTickPosition(QSlider::TicksBelow);
    _slider->setToolTip("Threshold value (0-255)");

    connect(_slider, &QSlider::valueChanged, this, [this](int value) {
        _thresholdValue = value;
        if (!_originalPixmap.isNull()) {
            applyThreshold(_originalPixmap);
        }
    });

    _layout->addWidget(_label);
    _layout->addWidget(_slider);
    _widget->setLayout(_layout);
}

unsigned int ThresholdModel::nPorts(PortType portType) const
{
    return (portType == PortType::In || portType == PortType::Out) ? 1 : 0;
}

bool ThresholdModel::eventFilter(QObject *object, QEvent *event)
{
    if (object == _label && event->type() == QEvent::Resize) {
        if (!_thresholdPixmap.isNull()) {
            _label->setPixmap(_thresholdPixmap.scaled(_label->size(), Qt::KeepAspectRatio));
        }
    }
    return false;
}

NodeDataType ThresholdModel::dataType(PortType const, PortIndex const) const
{
    return PixmapData().type();
}

std::shared_ptr<NodeData> ThresholdModel::outData(PortIndex)
{
    return std::make_shared<PixmapData>(_thresholdPixmap);
}

void ThresholdModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex const)
{
    if (auto d = std::dynamic_pointer_cast<PixmapData>(nodeData)) {
        _originalPixmap = d->pixmap();
        applyThreshold(_originalPixmap);
    }
}

void ThresholdModel::applyThreshold(const QPixmap &input)
{
    if (input.isNull()) return;

    QImage gray = input.toImage().convertToFormat(QImage::Format_Grayscale8);
    QImage binary(gray.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < gray.height(); ++y) {
        const uchar *srcLine = gray.constScanLine(y);
        uchar *dstLine = binary.scanLine(y);
        for (int x = 0; x < gray.width(); ++x) {
            dstLine[x] = (srcLine[x] >= _thresholdValue) ? 255 : 0;
        }
    }

    _thresholdPixmap = QPixmap::fromImage(binary);
    _label->setPixmap(_thresholdPixmap.scaled(_label->size(), Qt::KeepAspectRatio));
    _label->setToolTip(QString("Size: %1 x %2\nThreshold: %3")
                           .arg(_originalPixmap.width())
                           .arg(_originalPixmap.height())
                           .arg(_thresholdValue));

    Q_EMIT dataUpdated(0);
}

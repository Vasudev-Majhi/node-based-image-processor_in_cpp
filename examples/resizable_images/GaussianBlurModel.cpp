#include "GaussianBlurModel.hpp"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QToolTip>

GaussianBlurModel::GaussianBlurModel()
    : _label(new QLabel("Blurred Image will appear here")),
      _slider(new QSlider(Qt::Horizontal)),
      _widget(new QWidget),
      _layout(new QVBoxLayout(_widget))
{
    _label->setAlignment(Qt::AlignCenter);
    _label->setMinimumSize(200, 200);
    _label->installEventFilter(this);

    _slider->setMinimum(5);
    _slider->setMaximum(20);
    _slider->setValue(_blurRadius);
    _slider->setTickPosition(QSlider::TicksBelow);
    _slider->setToolTip("Adjust blur radius");

    connect(_slider, &QSlider::valueChanged, this, [this](int value) {
        _blurRadius = value;
        if (!_originalPixmap.isNull()) {
            applyGaussianBlur(_originalPixmap);
        }
    });

    _layout->addWidget(_label);
    _layout->addWidget(_slider);
    _widget->setLayout(_layout);
}

unsigned int GaussianBlurModel::nPorts(PortType portType) const
{
    return (portType == PortType::In || portType == PortType::Out) ? 1 : 0;
}

bool GaussianBlurModel::eventFilter(QObject *object, QEvent *event)
{
    if (object == _label && event->type() == QEvent::Resize) {
        if (!_blurredPixmap.isNull()) {
            _label->setPixmap(_blurredPixmap.scaled(_label->size(), Qt::KeepAspectRatio));
        }
    }
    return false;
}

NodeDataType GaussianBlurModel::dataType(PortType const, PortIndex const) const
{
    return PixmapData().type();
}

std::shared_ptr<NodeData> GaussianBlurModel::outData(PortIndex)
{
    return std::make_shared<PixmapData>(_blurredPixmap);
}

void GaussianBlurModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex const)
{
    if (auto d = std::dynamic_pointer_cast<PixmapData>(nodeData)) {
        _originalPixmap = d->pixmap();
        applyGaussianBlur(_originalPixmap);
    }
}

void GaussianBlurModel::applyGaussianBlur(const QPixmap &input)
{
    if (input.isNull()) return;

    QImage inputImage = input.toImage().convertToFormat(QImage::Format_ARGB32);
    QImage blurredImage(inputImage.size(), QImage::Format_ARGB32);
    blurredImage.fill(Qt::transparent);

    QPainter painter(&blurredImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(0, 0, inputImage);

    for (int i = 0; i < _blurRadius; ++i) {
        blurredImage = blurredImage.scaled(
            blurredImage.width() / 2,
            blurredImage.height() / 2,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
        blurredImage = blurredImage.scaled(
            inputImage.width(),
            inputImage.height(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
    }

    painter.end();

    _blurredPixmap = QPixmap::fromImage(blurredImage);
    _label->setPixmap(_blurredPixmap.scaled(_label->size(), Qt::KeepAspectRatio));
    _label->setToolTip(QString("Size: %1 x %2\nRadius: %3 px")
                           .arg(_originalPixmap.width())
                           .arg(_originalPixmap.height())
                           .arg(_blurRadius));

    Q_EMIT dataUpdated(0);
}

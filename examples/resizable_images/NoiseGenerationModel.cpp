#include "NoiseGenerationModel.hpp"
#include <QImage>
#include <QPainter>
#include <cmath>
#include <cstdlib>
#include <QRandomGenerator>
#include <random>


NoiseGenerationModel::NoiseGenerationModel()
{
    _widget = new QWidget;
    _previewLabel = new QLabel("Noise Preview");
    _previewLabel->setMinimumSize(200, 200);
    _previewLabel->setAlignment(Qt::AlignCenter);

    _noiseTypeCombo = new QComboBox;
    _noiseTypeCombo->addItems({"Perlin", "Simplex", "Worley"});

    _scaleSlider = new QSlider(Qt::Horizontal);
    _scaleSlider->setRange(1, 100);
    _scaleSlider->setValue(10);

    _octaveSlider = new QSlider(Qt::Horizontal);
    _octaveSlider->setRange(1, 8);
    _octaveSlider->setValue(4);

    _persistenceSlider = new QSlider(Qt::Horizontal);
    _persistenceSlider->setRange(0, 100);
    _persistenceSlider->setValue(50);

    _displacementCheck = new QCheckBox("Displacement Map Output");

    auto* form = new QFormLayout;
    form->addRow("Noise Type:", _noiseTypeCombo);
    form->addRow("Scale:", _scaleSlider);
    form->addRow("Octaves:", _octaveSlider);
    form->addRow("Persistence:", _persistenceSlider);
    form->addRow(_displacementCheck);

    auto* layout = new QVBoxLayout(_widget);
    layout->addLayout(form);
    layout->addWidget(_previewLabel);

    connect(_noiseTypeCombo, &QComboBox::currentTextChanged, this, &NoiseGenerationModel::generateNoise);
    connect(_scaleSlider, &QSlider::valueChanged, this, &NoiseGenerationModel::generateNoise);
    connect(_octaveSlider, &QSlider::valueChanged, this, &NoiseGenerationModel::generateNoise);
    connect(_persistenceSlider, &QSlider::valueChanged, this, &NoiseGenerationModel::generateNoise);
    connect(_displacementCheck, &QCheckBox::toggled, this, &NoiseGenerationModel::generateNoise);

    generateNoise();
}

unsigned int NoiseGenerationModel::nPorts(QtNodes::PortType portType) const
{
    return (portType == QtNodes::PortType::Out) ? 1 : 0;
}

QtNodes::NodeDataType NoiseGenerationModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const
{
    return PixmapData().type();
}

std::shared_ptr<QtNodes::NodeData> NoiseGenerationModel::outData(QtNodes::PortIndex)
{
    return std::make_shared<PixmapData>(_pixmap);
}

void NoiseGenerationModel::generateNoise()
{
    const int width = 256;
    const int height = 256;
    const QString type = _noiseTypeCombo->currentText();

    if (type == "Perlin") {
        float scale = _scaleSlider->value() / 10.0f;
        int octaves = _octaveSlider->value();
        float persistence = _persistenceSlider->value() / 100.0f;
        QImage img = generatePerlinNoise(width, height, scale, octaves, persistence);
        _pixmap = QPixmap::fromImage(img);
    } else {
        QImage img = generateMockNoise(width, height);
        _pixmap = QPixmap::fromImage(img);
    }

    _previewLabel->setPixmap(_pixmap.scaled(_previewLabel->size(), Qt::KeepAspectRatio));
    Q_EMIT dataUpdated(0);
}

QImage NoiseGenerationModel::generateMockNoise(int width, int height)
{
    QImage img(width, height, QImage::Format_Grayscale8);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
           img.setPixel(x, y, qRgb(QRandomGenerator::global()->bounded(256), 0, 0));

    return img;
}

// Basic Perlin noise function
static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static float grad(int hash, float x, float y) {
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

QImage NoiseGenerationModel::generatePerlinNoise(int width, int height, float scale, int octaves, float persistence)
{
    QImage img(width, height, QImage::Format_Grayscale8);
    std::vector<int> p(512);
    for (int i = 0; i < 256; ++i) p[i] = i;
    // std::random_shuffle(p.begin(), p.begin() + 256);
    std::shuffle(p.begin(), p.begin() + 256, std::mt19937{std::random_device{}()});

    for (int i = 0; i < 256; ++i) p[256 + i] = p[i];

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float xf = x / scale;
            float yf = y / scale;
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float noise = 0.0f;

            for (int o = 0; o < octaves; ++o) {
                int X = (int)floor(xf * frequency) & 255;
                int Y = (int)floor(yf * frequency) & 255;

                float x0 = xf * frequency - floor(xf * frequency);
                float y0 = yf * frequency - floor(yf * frequency);

                float u = fade(x0);
                float v = fade(y0);

                int aa = p[p[X] + Y];
                int ab = p[p[X] + Y + 1];
                int ba = p[p[X + 1] + Y];
                int bb = p[p[X + 1] + Y + 1];

                float grad00 = grad(aa, x0, y0);
                float grad01 = grad(ab, x0, y0 - 1);
                float grad10 = grad(ba, x0 - 1, y0);
                float grad11 = grad(bb, x0 - 1, y0 - 1);

                float x1 = lerp(grad00, grad10, u);
                float x2 = lerp(grad01, grad11, u);
                float result = lerp(x1, x2, v);

                noise += result * amplitude;
                amplitude *= persistence;
                frequency *= 2.0f;
            }

            int color = static_cast<int>((noise + 1.0f) * 127.5f);
            img.setPixel(x, y, qRgb(color, color, color));
        }
    }

    return img;
}

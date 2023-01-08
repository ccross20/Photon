#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QResizeEvent>
#include "huebox.h"

namespace photon {

struct pixValue{
    uchar b;
    uchar g;
    uchar r;
    uchar a;
};

ColorBox::ColorBox(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(25,25);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
}


QColor ColorBox::value() const
{
    return m_color;
}

void ColorBox::setValue(const QColor &value)
{
    m_color = value.convertTo(QColor::Hsv);

    renderBox();
    update();
}


void ColorBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawImage(0,0,m_gradientImage);
}

void ColorBox::resizeEvent(QResizeEvent *event)
{
    m_gradientImage = QImage(event->size(),QImage::Format_ARGB32_Premultiplied);
    renderBox();
}

void ColorBox::renderBox()
{
    pixValue* rasterPixel = reinterpret_cast<pixValue*>(m_gradientImage.bits());
    uint w = static_cast<uint>(m_gradientImage.width());
    uint h = static_cast<uint>(m_gradientImage.height());

    double wInc = 1.0/w;
    double hInc = 1.0/h;
    float hue,s,v;
    m_color.getHsvF(&hue, &s, &v);

    //hue *= 360.0f;
    float r,g,b;


    for (uint y = 0; y < h; ++y) {
        for (uint x = 0; x < w; ++x) {

            QColor color = QColor::fromHsvF(hue, x * wInc, 1.0- (y * hInc));
            color = color.convertTo(QColor::Rgb);
            color.getRgbF(&r, &g, &b);

            (*rasterPixel).b = b*255;
            (*rasterPixel).g = g*255;
            (*rasterPixel).r = r*255;
            (*rasterPixel).a = 255;

            rasterPixel++;
        }
    }

}

void ColorBox::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mousePressEvent(event);
    emit beginEditing();

    float h,s,v;
    m_color.getHsvF(&h, &s, &v);

    QPointF pos(event->pos());
    m_color.setHsvF(h, qMin(1.0, qMax(pos.x()/m_gradientImage.width(),0.0)), 1.0 - qMin(1.0, qMax(pos.y()/m_gradientImage.height(),0.0)));

    emit colorChange(m_color);
}

void ColorBox::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseMoveEvent(event);

    float h,s,v;
    m_color.getHsvF(&h, &s, &v);

    QPointF pos(event->pos());
    m_color.setHsvF(h, qMin(1.0, qMax(pos.x()/m_gradientImage.width(),0.0)), 1.0 - qMin(1.0, qMax(pos.y()/m_gradientImage.height(),0.0)));

    emit colorChange(m_color);
}



void ColorBox::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit endEditing();
}

HueBox::HueBox(QWidget *parent) : QWidget(parent)
{
    m_colorBox = new ColorBox;

    connect(m_colorBox, SIGNAL(colorChange(QColor)), this, SLOT(boxColorChange(QColor)));
    connect(m_colorBox, SIGNAL(beginEditing()), this, SIGNAL(beginEditing()));
    connect(m_colorBox, SIGNAL(endEditing()), this, SIGNAL(endEditing()));

    m_hueSlider = new ColorSlider(Qt::red, ColorSlider::Mode_HSVA_Hue);
    m_hueSlider->setTrackRadius(0);

    connect(m_hueSlider, SIGNAL(valueChange(double)), this, SLOT(hueSliderChange(double)));
    connect(m_hueSlider, SIGNAL(beginEditing()), this, SIGNAL(beginEditing()));
    connect(m_hueSlider, SIGNAL(endEditing()), this, SIGNAL(endEditing()));

    m_alphaSlider = new ColorSlider(Qt::red, ColorSlider::Mode_RGBA_Alpha);
    m_alphaSlider->setTrackRadius(0);
    m_alphaSlider->setOrientation(Qt::Vertical);

    connect(m_alphaSlider, SIGNAL(valueChange(double)), this, SLOT(alphaSliderChange(double)));
    connect(m_alphaSlider, SIGNAL(beginEditing()), this, SIGNAL(beginEditing()));
    connect(m_alphaSlider, SIGNAL(endEditing()), this, SIGNAL(endEditing()));

    QHBoxLayout *hLayout = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(2);
    vLayout->addWidget(m_colorBox);
    vLayout->addWidget(m_hueSlider);

    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(2);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(m_alphaSlider);

    setLayout(hLayout);
}

QColor HueBox::value() const
{
    return m_colorBox->value();
}

void HueBox::setShowAlpha(bool value)
{
    if(m_showAlpha == value)
        return;

    m_showAlpha = value;
    m_alphaSlider->setVisible(value);
    update();
}

bool HueBox::showAlpha() const
{
    return m_showAlpha;
}

void HueBox::setValue(const QColor &value)
{

    m_colorBox->blockSignals(true);
    m_hueSlider->blockSignals(true);
    m_alphaSlider->blockSignals(true);
    m_colorBox->setValue(value);
    m_alphaSlider->setColor(value);
    m_hueSlider->setColor(value);
    m_colorBox->blockSignals(false);
    m_hueSlider->blockSignals(false);
    m_alphaSlider->blockSignals(false);
}

void HueBox::boxColorChange(QColor color)
{
    emit colorChange(color);
}

void HueBox::hueSliderChange(double value)
{
    m_colorBox->blockSignals(true);

    QColor c = m_colorBox->value();

    float h,s,v;

    c.getHsvF(&h, &s, &v);
    c.setHsvF(value, s, v);

    m_colorBox->setValue(c);
    m_colorBox->blockSignals(false);
    emit colorChange(c);
}

void HueBox::alphaSliderChange(double value)
{
    m_colorBox->blockSignals(true);

    QColor c = m_colorBox->value();
    c.setAlpha(value);
    //qDebug() << value;

    m_colorBox->setValue(c);
    m_colorBox->blockSignals(false);
    emit colorChange(c);
}


} // namespace exo

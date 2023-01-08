#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "colorslider.h"

namespace photon {

ColorSlider::ColorSlider(const QColor &color, ColorMode mode, double value, QWidget *parent) : QWidget (parent),
    m_trackInset(QSize(3,3)),
    m_value(value),
    m_mode(mode)
{

    setMinimumHeight(20);
    setMaximumHeight(20);
    setMinimumWidth(40);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));

    int brushWidth = 6;
    int halfWidth = 3;
    QPixmap alphaPix = QPixmap{brushWidth,brushWidth};
    alphaPix.fill(Qt::white);
    QPainter pPainter(&alphaPix);
    pPainter.setPen(Qt::NoPen);
    pPainter.setBrush(Qt::black);
    pPainter.drawRect(0,0,halfWidth,halfWidth);
    pPainter.drawRect(halfWidth,halfWidth,halfWidth,halfWidth);
    m_alphaBGBrush = QBrush{alphaPix};

    setColor(color);
}

ColorSlider::ColorSlider(ColorMode mode, double value, QWidget *parent):QWidget (parent),
    m_value(value),
    m_mode(mode)
{

}

void ColorSlider::setTrackRadius(int value)
{
    if(m_trackRadius == value)
        return;
    m_trackRadius = value;
    if(m_trackRadius < 0)
        m_calculatedRadius = m_trackRect.height()/2;
    update();
}

int ColorSlider::trackRadius() const
{
    return m_trackRadius;
}

void ColorSlider::setOrientation(Qt::Orientation orientation)
{
    if(m_orientation == orientation)
        return;
    m_orientation = orientation;

    if(orientation == Qt::Horizontal)
    {
        setMinimumHeight(20);
        setMaximumHeight(20);
        setMinimumWidth(40);
        setMaximumWidth(16777215);
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
    }
    else
    {
        setMinimumWidth(20);
        setMaximumWidth(20);
        setMinimumHeight(40);
        setMaximumHeight(16777215);
        setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding));
    }

    update();
}

Qt::Orientation ColorSlider::orientation() const
{
    return m_orientation;
}

double ColorSlider::value() const
{
    return m_value;
}

void ColorSlider::setValue(const double &value)
{
    if(qFuzzyCompare(value, m_value))
        return;
    if(m_value < 0.0)
        m_value = 0.0;
    else if(m_value > 1.0)
        m_value = 1.0;

    m_value = value;
    update();
}

void ColorSlider::setColors(const QColor &colorA, const QColor &colorB)
{

    m_colorA = colorA;
    m_colorB = colorB;

    switch (m_mode) {
        case photon::ColorSlider::Mode_RGBA_Red:
        case photon::ColorSlider::Mode_RGBA_Blue:
        case photon::ColorSlider::Mode_RGBA_Green:
        case photon::ColorSlider::Mode_RGBA_Alpha:
            m_colorA = m_colorA.convertTo(QColor::Rgb);
            m_colorB = m_colorB.convertTo(QColor::Rgb);
            break;
        case photon::ColorSlider::Mode_HSVA_Hue:
        case photon::ColorSlider::Mode_HSVA_Saturation:
        case photon::ColorSlider::Mode_HSVA_Value:
        case photon::ColorSlider::Mode_HSVA_Alpha:
            m_colorA = m_colorA.convertTo(QColor::Hsv);
            m_colorB = m_colorB.convertTo(QColor::Hsv);
            break;
        case photon::ColorSlider::Mode_HSLA_Hue:
        case photon::ColorSlider::Mode_HSLA_Saturation:
        case photon::ColorSlider::Mode_HSLA_Lightness:
        case photon::ColorSlider::Mode_HSLA_Alpha:
        m_colorA = m_colorA.convertTo(QColor::Hsl);
        m_colorB = m_colorB.convertTo(QColor::Hsl);
            break;
    }

    if(m_mode == Mode_RGBA_Alpha)
    {
        m_colorA.setAlpha(0.0);
        m_colorB.setAlpha(1.0);
    }

    update();
}

void ColorSlider::setColor(const QColor &color)
{
    m_colorA = color;

    switch (m_mode) {
        case photon::ColorSlider::Mode_RGBA_Red:
        case photon::ColorSlider::Mode_RGBA_Blue:
        case photon::ColorSlider::Mode_RGBA_Green:
        case photon::ColorSlider::Mode_RGBA_Alpha:
            m_colorA = m_colorA.convertTo(QColor::Rgb);
            break;
        case photon::ColorSlider::Mode_HSVA_Hue:
        case photon::ColorSlider::Mode_HSVA_Saturation:
        case photon::ColorSlider::Mode_HSVA_Value:
        case photon::ColorSlider::Mode_HSVA_Alpha:
            m_colorA = m_colorA.convertTo(QColor::Hsv);
            break;
        case photon::ColorSlider::Mode_HSLA_Hue:
        case photon::ColorSlider::Mode_HSLA_Saturation:
        case photon::ColorSlider::Mode_HSLA_Lightness:
        case photon::ColorSlider::Mode_HSLA_Alpha:
            m_colorA = m_colorA.convertTo(QColor::Hsl);
            break;
    }


    m_colorB = m_colorA;

    m_colorA.setAlphaF(1.0);
    m_colorB.setAlphaF(1.0);

    switch (m_mode) {
    case photon::ColorSlider::Mode_RGBA_Red:
        m_value = color.redF();
        m_colorA.setRedF(0.0);
        m_colorB.setRedF(1.0);
        break;
    case photon::ColorSlider::Mode_RGBA_Blue:
        m_value = color.blueF();
        m_colorA.setBlueF(0.0);
        m_colorB.setBlueF(1.0);
        break;
    case photon::ColorSlider::Mode_RGBA_Green:
        m_value = color.greenF();
        m_colorA.setGreenF(0.0);
        m_colorB.setGreenF(1.0);
        break;
    case photon::ColorSlider::Mode_RGBA_Alpha:
        m_value = color.alphaF();
        m_colorA.setAlphaF(0.0);
        m_colorB.setAlphaF(1.0);
        break;
    case photon::ColorSlider::Mode_HSVA_Hue:
    {
        float h,s,v;
        m_colorA.getHsvF(&h, &s, &v);
        m_value = h;
        m_colorA.setHsvF(0.0, s, v);
        m_colorB.setHsvF(1.0, s, v);
    }
        break;
    case photon::ColorSlider::Mode_HSVA_Saturation:
    {
        float h,s,v;
        m_colorA.getHsvF(&h, &s, &v);
        m_value = s;
        m_colorA.setHsvF(h, 0.0, v);
        m_colorB.setHsvF(h, 1.0, v);
    }
        break;
    case photon::ColorSlider::Mode_HSVA_Value:
    {
        float h,s,v;
        m_colorA.getHsvF(&h, &s, &v);
        m_value = v;
        m_colorA.setHsvF(h, s, 0.0);
        m_colorB.setHsvF(h, s, 1.0);
    }
        break;
    case photon::ColorSlider::Mode_HSVA_Alpha:
    case photon::ColorSlider::Mode_HSLA_Alpha:
        m_value = color.alphaF();
        m_colorA.setAlpha(0.0);
        break;
    case photon::ColorSlider::Mode_HSLA_Hue:
    {
        float h,s,l;
        m_colorA.getHslF(&h, &s, &l);
        m_value = h;
        m_colorA.setHslF(0.0, s, l);
        m_colorB.setHslF(1.0, s, l);
    }
        break;
    case photon::ColorSlider::Mode_HSLA_Saturation:
    {
        float h,s,l;
        m_colorA.getHslF(&h, &s, &l);
        m_value = s;
        m_colorA.setHslF(h, 0.0, l);
        m_colorB.setHslF(h, 1.0, l);
    }
        break;
    case photon::ColorSlider::Mode_HSLA_Lightness:
    {
        float h,s,l;
        m_colorA.getHslF(&h, &s, &l);
        m_value = l;
        m_colorA.setHslF(h, s, 0.0);
        m_colorB.setHslF(h, s, 0.5);
        m_colorC.setHslF(h, s, 1.0);
    }
        break;

    }

    update();
}

void ColorSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);



    if(m_colorA.alpha() < 255 || m_colorB.alpha() < 255)
    {
        painter.setBrush(m_alphaBGBrush);
        painter.drawRoundedRect(m_trackRect, m_calculatedRadius, m_calculatedRadius);
    }

    QLinearGradient gradient;

    if(m_orientation == Qt::Horizontal)
        gradient = QLinearGradient(m_trackRect.topLeft(), m_trackRect.topRight());
    else
        gradient = QLinearGradient(m_trackRect.bottomLeft(), m_trackRect.topLeft());

    if(m_mode == ColorMode::Mode_HSVA_Hue || m_mode == ColorMode::Mode_HSLA_Hue)
    {
        const int hue_stops = 24;
        if ( gradient.stops().size() < hue_stops )
        {
            for ( double a = 0; a < 1.0; a+=1.0/(hue_stops-1) )
                gradient.setColorAt(a,QColor::fromHsvF(a,1.0,1.0,1.0));
            gradient.setColorAt(1,QColor::fromHsvF(0.0,1.0,1.0,1.0));
        }
    }
    else if(m_mode == ColorMode::Mode_HSLA_Lightness)
    {
        gradient.setColorAt(0,m_colorA.convertTo(QColor::Rgb));
        gradient.setColorAt(.5,m_colorB.convertTo(QColor::Rgb));
        gradient.setColorAt(1,m_colorC.convertTo(QColor::Rgb));
    }
    else
    {        
        gradient.setColorAt(0,m_colorA.convertTo(QColor::Rgb));
        gradient.setColorAt(1,m_colorB.convertTo(QColor::Rgb));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gradient));
    painter.drawRoundedRect(m_trackRect, m_calculatedRadius, m_calculatedRadius);

    painter.setBrush(Qt::NoBrush);

    if(m_orientation == Qt::Horizontal)
    {
        QRect handleRect((m_value * (m_trackRect.width() - (m_calculatedRadius * 2))) + m_trackRect.left() + m_calculatedRadius-1,0,1,height());
        painter.fillRect(handleRect, Qt::white);
        painter.fillRect(handleRect.translated(1,0), Qt::black);
    }
    else
    {
        QRect handleRect(0,((1.0 - m_value) * (m_trackRect.height() - (m_calculatedRadius * 2))) + m_trackRect.top() + m_calculatedRadius-1,width(),1);
        painter.fillRect(handleRect, Qt::white);
        painter.fillRect(handleRect.translated(0,1), Qt::black);
    }


}

void ColorSlider::handleMouse(const QPoint &pt)
{

    if(m_orientation == Qt::Horizontal)
        m_value = (pt.x() -(m_trackRect.left() + m_trackRadius))/static_cast<double>(m_trackRect.width() - (m_trackRadius * 2));
    else
        m_value = (pt.y() -(m_trackRect.top() + m_trackRadius))/static_cast<double>(m_trackRect.height() - (m_trackRadius * 2));

    if(m_value < 0.0)
        m_value = 0.0;
    else if(m_value > 1.0)
        m_value = 1.0;

    if(m_orientation == Qt::Vertical)
        m_value = 1.0 - m_value;

    emit valueChange(m_value);

    /*
    switch (m_mode) {
        case photon::ColorSlider::Mode_RGBA_Red:
            setValue(QColor::fromRgbF(m_value,0,0,1));
            break;
        case photon::ColorSlider::Mode_RGBA_Blue:
            setValue(QColor::fromRgbF(0,m_value,0,1));
            break;
        case photon::ColorSlider::Mode_RGBA_Green:
            setValue(QColor::fromRgbF(0,0,m_value,1));
            break;
        case photon::ColorSlider::Mode_RGBA_Alpha:
            setValue(QColor::fromRgbF(m_colorA.redF(),m_colorA.blueF(),m_colorA.greenF(),m_value));
            break;
    }
    */

    update();
}

void ColorSlider::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mousePressEvent(event);
    emit beginEditing();
    setFocus();
    handleMouse(event->pos());
}

void ColorSlider::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseMoveEvent(event);
    handleMouse(event->pos());
}

void ColorSlider::mouseReleaseEvent(QMouseEvent *event)
{

    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseReleaseEvent(event);
    emit endEditing();
}

void ColorSlider::resizeEvent(QResizeEvent *event)
{
    m_trackRect = rect().adjusted(m_trackInset.width(),m_trackInset.height(),-m_trackInset.width(),-m_trackInset.height());

    if(m_trackRadius < 0)
    {
        if(m_orientation == Qt::Horizontal)
            m_calculatedRadius = m_trackRect.height()/2;
        else
            m_calculatedRadius = m_trackRect.width()/2;
    }
}


} // namespace exo

#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <qmath.h>
#include <QDebug>



#include "colorwheel_p.h"

namespace exo {

ColorWheel::ColorWheel(QWidget *parent) : QWidget(parent)
{
    m_triangle = new QImage(m_radius*2,m_radius*2, QImage::Format_ARGB32);
    m_hue = 0.0;
    //setMinimumSize(200,200);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));

}

ColorWheel::~ColorWheel()
{
    delete m_triangle;
}

Color ColorWheel::value() const
{
    Color c{m_hue/360.0, m_saturation, m_lightness,1.0, Color::ColorFormatHSLA};
    return c;
}

void ColorWheel::setValue(const Color &value)
{
    //QColor color = QColor::fromRgbF(value.red(), value.blue(), value.green(), value.alpha());
    Color c = value.convertedTo(Color::ColorFormatHSLA);
    m_hue = c.getHue() * 360.0;
    m_saturation = c.getSaturation();
    m_lightness = c.getLightness();

    //qDebug() << "set Value saturation: " << m_saturation << "   lightness: " << m_lightness;


    //m_hue = 0.0;

    if(m_hue < 0)
        m_hue += 360.0;

    //m_triangleTransform.rotate(m_hue);
    renderTriangle();

    update();
}

void ColorWheel::renderTriangle()
{

    if(m_radius <= 0.0)
        return;

    //m_triangle->fill(QColor(40,40,40,0));
    m_triangle->fill(Qt::transparent);

    pixfmt pf(m_buffer);
    base_ren_type ren_base(pf);

    QPointF triCenter = m_triangle->rect().center();

    const double arc = (M_PI*2.0)/3.0;
    const double arrowRotation = 0.0;

    double r,g,b;
    Color::hslToRGB(m_hue, 1.0, .5, &r, &g, &b);

    m_trianglePoly.clear();
    m_trianglePoly.append(QPointF(triCenter.x()+m_innerRadius*cos(arrowRotation),triCenter.y()+m_innerRadius*sin(arrowRotation)));
    m_trianglePoly.append(QPointF(triCenter.x()+m_innerRadius*cos(arc+arrowRotation),triCenter.y()+m_innerRadius*sin(arc+arrowRotation)));
    m_trianglePoly.append(QPointF(triCenter.x()+m_innerRadius*cos(arc*2.0+arrowRotation),triCenter.y()+m_innerRadius*sin(arc*2.0+arrowRotation)));

    m_triHeight = m_innerRadius*sin(arc);
    m_shortX = m_innerRadius*cos(arc);
    // Single triangle
    m_span_gen.colors(rgba(r,   g,   b,  1),
                    rgba(0,   0,   0,  1),
                    rgba(1,   1,   1,  1)
                    );
    m_span_gen.triangle(m_trianglePoly.at(0).x(),m_trianglePoly.at(0).y(),
                      m_trianglePoly.at(1).x(),m_trianglePoly.at(1).y(),
                      m_trianglePoly.at(2).x(),m_trianglePoly.at(2).y(), .5);
    m_ras.add_path(m_span_gen);

    render_scanlines_aa(m_ras, m_sl, ren_base, m_span_alloc, m_span_gen);
}

void ColorWheel::renderWheel()
{
    m_wheel = QPixmap(m_radius*2,m_radius*2);
    m_wheel.fill(Qt::transparent);
    QPainter painter(&m_wheel);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    const int hue_stops = 24;
    QConicalGradient gradient_hue(0, 0, 0);
    if ( gradient_hue.stops().size() < hue_stops )
    {
        for ( double a = 1.0; a > 0.0; a-=1.0/(hue_stops-1) )
            gradient_hue.setColorAt(a,QColor::fromHsvF(a,1.0,1.0,1.0));
        gradient_hue.setColorAt(1,QColor::fromHsvF(0.0,1.0,1.0,1.0));
    }

    painter.translate(m_radius,m_radius);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gradient_hue));
    painter.drawEllipse(QPointF(0,0),m_radius,m_radius);


    painter.setBrush(Qt::transparent);//palette().background());
    painter.drawEllipse(QPointF(0,0),m_innerRadius,m_innerRadius);

}

void ColorWheel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //painter.fillRect(event->rect(),QColor(40,40,40,255));

    // hue wheel
    QPointF wheelOffset = m_center - m_wheel.rect().center();
    painter.drawPixmap(wheelOffset, m_wheel);

    double dist = ((m_radius - m_innerRadius)/2.0)+m_innerRadius;

    double radHue = (360.0 - m_hue) * (M_PI/180.0);

    QPointF circlePt((cos(radHue)*dist)+m_radius+wheelOffset.x(),(sin(radHue)*dist)+m_radius + wheelOffset.y());

    painter.setPen(Qt::NoPen);

    int circleRadius = static_cast<int>(m_radius *.08);

    painter.setBrush(palette().color(QPalette::Base));
    painter.drawEllipse(circlePt, circleRadius+2, circleRadius+2);

    painter.setBrush(QColor::fromHslF(m_hue/360.0, 1.0, .5, 1.0));
    painter.drawEllipse(circlePt, circleRadius, circleRadius);
    painter.drawImage(m_center - m_triangle->rect().center(), *m_triangle);

    /*
    qDebug() << "Sat: " << m_saturation << "  light: " << m_lightness;

    QPointF triPoint = m_center + QPointF((m_saturation*(m_innerRadius - m_shortX)) + m_shortX, ((1.0-m_lightness)*(m_triHeight*2.0)) - m_triHeight);
    //QPointF triPoint = m_center + QPointF((m_saturation*(m_innerRadius - m_shortX)) + m_shortX, ((1.0-m_lightness)*(m_triHeight*2.0)) - m_triHeight);

    painter.setBrush(palette().background());
    painter.drawEllipse(triPoint, 6, 6);

    painter.setBrush(QColor::fromHslF(m_hue/360.0, m_saturation, m_lightness, 1.0));
    painter.drawEllipse(triPoint, 4, 4);
    */

}

void ColorWheel::resizeEvent(QResizeEvent *event)
{
    if(event->size().height() < event->size().width())
        m_radius = (event->size().height()/2.0)-10.0;
    else
        m_radius = (event->size().width()/2.0)-10.0;

    //m_center = QPointF(m_radius + 10.0, m_radius + 10.0);
    m_center = QPointF(event->size().width()/2,event->size().height()/2);

    m_innerRadius = m_radius*.9;

    delete m_triangle;
    m_triangle = new QImage(m_radius*2,m_radius*2, QImage::Format_ARGB32);
    m_triangle->fill(Qt::transparent);
    m_buffer.attach(m_triangle->bits(), m_triangle->width(), m_triangle->height(), m_triangle->width() * 4);

    renderWheel();
    renderTriangle();
}

void ColorWheel::wheelPress(const QPointF &pt)
{
    m_interaction = InteractionWheel;
    double result = atan2(pt.y(), pt.x());

    result *= 180.0/M_PI;

    if(result < 0)
        result += 360;

    m_hue = 360.0 - result;
    renderTriangle();

    update();

    Color c{m_hue/360.0, m_saturation, m_lightness, 1.0, Color::ColorFormatHSLA};
    emit colorChange(c);

}

void ColorWheel::trianglePress(const QPointF &pt)
{
    m_interaction = InteractionTriangle;

    const QPointF p = pt;

    m_saturation = qMax(qMin((p.x() - m_shortX)/(m_innerRadius-m_shortX), 1.0),0.0);
    //m_lightness = 1.0 - qMax(qMin((p.y() + m_triHeight)/(m_triHeight*2.0), 1.0),0.0);
    m_lightness = 1.0 - qMax(qMin((p.y() + m_triHeight)/(m_triHeight*2.0), .5 + ((1.0 - m_saturation)*.5)),.5 - ((1.0 - m_saturation)*.5));


    //qDebug() << "saturation: " << m_saturation << "   lightness: " << m_lightness;

    update();

    Color c{m_hue/360.0, m_saturation, m_lightness, 1.0, Color::ColorFormatHSLA};
    emit colorChange(c);
}

void ColorWheel::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mousePressEvent(event);
    QPointF center(width()/2,height()/2);

    QPointF p = event->pos() - center;

    QLineF line(center,event->pos());


    if(line.length() < m_radius && line.length() > m_innerRadius)
    {
        wheelPress(p);
    }
    else if(m_trianglePoly.containsPoint(event->pos() - ( center - m_triangle->rect().center()),Qt::FillRule::WindingFill))
    {
        trianglePress(p);
    }
    emit beginEditing();
}

void ColorWheel::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseMoveEvent(event);
    QPointF center(width()/2,height()/2);
    QPointF p = event->pos() - center;

    switch (m_interaction) {
        case InteractionWheel:
            wheelPress(p);
            break;
        case InteractionTriangle:
            trianglePress(p);
            break;
        default:
            break;
    }

}

void ColorWheel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseReleaseEvent(event);
    Q_UNUSED(event)
    m_interaction = InteractionNone;
    emit endEditing();
}

} // namespace exo

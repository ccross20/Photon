#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include "colorwheelswatch.h"
#include "colorselectordialog.h"

namespace photon {

class ColorWheelSwatch::Impl
{
public:
    Impl(const QColor &color);
    void renderWheel();

    QColor color;
    int maxWidth = 100;
    QPixmap wheel;
};

ColorWheelSwatch::Impl::Impl(const QColor &color) : color(color)
{

    renderWheel();
}

ColorWheelSwatch::ColorWheelSwatch(const QColor &color, QWidget *parent) : QWidget(parent), m_impl(new Impl(color))
{
    setMinimumSize(50,24);
}

ColorWheelSwatch::~ColorWheelSwatch()
{
    delete m_impl;
}

const QColor &ColorWheelSwatch::color() const
{
    return m_impl->color;
}

void ColorWheelSwatch::setColor(const QColor &color)
{
    if(m_impl->color == color)
        return;
    m_impl->color = color;

    //if(color.format == Color::ColorFormatCMYKA)
        //qDebug() << color;
    emit colorChanged(color);
    update();
}

void ColorWheelSwatch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;

    int cutoutDiameter = 22;
    QRect r = rect();
    /*
    if(r.width() > m_impl->maxWidth)
    {
        r.setX(r.width()-m_impl->maxWidth);
        r.setWidth(m_impl->maxWidth);
    }
    */
    //r.adjust(0,0,0,0);
    int halfDiameter = cutoutDiameter/2;
    path.addRect(r);
    QPainterPath circlePath;
    circlePath.addEllipse(QRect(-2,(height()-cutoutDiameter)+2,cutoutDiameter,cutoutDiameter));

    QPainterPath rectPath;
    rectPath.addRect(0,height()-halfDiameter,halfDiameter,halfDiameter);

    path = path.subtracted(circlePath);
    path = path.subtracted(rectPath);
    /*
    if(m_impl->color.alpha() < 1.0)
        p.fillPath(path, exoApp->settings()->alphaBackgroundBrush());
        */
    p.fillPath(path, QBrush(m_impl->color));

    if(isEnabled())
    {

        p.drawPixmap(0,height()-m_impl->wheel.height(),m_impl->wheel);
    }
    else
    {

        //p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        /*
        p.fillPath(path, exoApp->settings()->createHashBrush(palette().window().color(),QSize{20,20},8));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawPixmap(0,height()-m_impl->wheel.height(),QDrawingUtils::tintPixmap(m_impl->wheel, Qt::gray));
        */
    }

}

void ColorWheelSwatch::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit beganEditing();
    emit swatchClicked();

    //ColorSelectorDialog *colorsWidget = new ColorSelectorDialog(m_impl->color, window());
    ColorSelectorDialog *colorsWidget = new ColorSelectorDialog(m_impl->color, nullptr);
    colorsWidget->setAttribute(Qt::WA_DeleteOnClose);
    colorsWidget->show();
    colorsWidget->raise();
    colorsWidget->activateWindow();

    connect(colorsWidget,SIGNAL(selectionChanged(QColor)),this,SLOT(setColor(QColor)));
    //connect(colorsWidget,SIGNAL(dialogAccepted()),this,SIGNAL(finishedEditing()));
    //connect(colorsWidget,SIGNAL(dialogRejected()),this,SIGNAL(finishedEditing()));


}

void ColorWheelSwatch::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ColorWheelSwatch::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ColorWheelSwatch::Impl::renderWheel()
{
    int radius = 9;
    int innerRadius = radius-4;

    wheel = QPixmap(radius*2,radius*2);
    wheel.fill(Qt::transparent);
    QPainter painter(&wheel);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);


    const int hue_stops = 24;
    QConicalGradient gradient_hue(0, 0, 0);
    if ( gradient_hue.stops().size() < hue_stops )
    {
        for ( double a = 0; a < 1.0; a+=1.0/(hue_stops-1) )
        {
            gradient_hue.setColorAt(1.0-a,QColor::fromHsvF(a,1.0,1.0,1.0));
        }
        gradient_hue.setColorAt(1,QColor::fromHsvF(0.0,1.0,1.0,1.0));
    }

    painter.translate(radius,radius);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(gradient_hue));
    painter.drawEllipse(QPointF(0,0),radius,radius);

    painter.setBrush(Qt::transparent);//palette().background());
    painter.drawEllipse(QPointF(0,0),innerRadius,innerRadius);

}


} // namespace photon

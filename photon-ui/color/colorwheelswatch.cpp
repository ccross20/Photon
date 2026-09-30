#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <cmath>
#include "colorwheelswatch.h"
#include "colorselectordialog.h"

namespace photon {

class ColorWheelSwatch::Impl
{
public:
    Impl(const QColor &color);
    void renderWheel();

    void openPicker(ColorWheelSwatch *swatch);

    QColor color;
    int maxWidth = 100;
    QPixmap wheel;

    // Press-drag-release tracking: a drag shifts the hue, a plain click opens
    // the picker.
    QPoint pressPos;
    QColor pressColor;
    bool pressed = false;
    bool dragging = false;
};

namespace {
// Hue change per pixel of horizontal drag - a 180px sweep covers the wheel.
constexpr double kDegreesPerPixel = 2.0;
}

void ColorWheelSwatch::Impl::openPicker(ColorWheelSwatch *t_swatch)
{
    ColorSelectorDialog *colorsWidget = new ColorSelectorDialog(color, nullptr);
    colorsWidget->setAttribute(Qt::WA_DeleteOnClose);
    colorsWidget->show();
    colorsWidget->raise();
    colorsWidget->activateWindow();

    QObject::connect(colorsWidget, SIGNAL(selectionChanged(QColor)), t_swatch, SLOT(setColor(QColor)));
}

ColorWheelSwatch::Impl::Impl(const QColor &color) : color(color)
{

    renderWheel();
}

ColorWheelSwatch::ColorWheelSwatch(const QColor &color, QWidget *parent) : QWidget(parent), m_impl(new Impl(color))
{
    setMinimumSize(50,24);
    setToolTip("Click to open the color picker\nDrag left or right to change the hue");
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
    if(event->button() != Qt::LeftButton)
        return;
    m_impl->pressed = true;
    m_impl->dragging = false;
    m_impl->pressPos = event->pos();
    m_impl->pressColor = m_impl->color;
}

void ColorWheelSwatch::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_impl->pressed)
        return;

    const int dx = event->pos().x() - m_impl->pressPos.x();
    if(!m_impl->dragging)
    {
        if((event->pos() - m_impl->pressPos).manhattanLength() < QApplication::startDragDistance())
            return;
        m_impl->dragging = true;
        setCursor(Qt::SizeHorCursor);
        emit beganEditing();
    }

    // A grey/white/black starting colour has no hue to rotate, so the drag
    // starts it from full saturation and brightness instead.
    const QColor start = m_impl->pressColor.toHsv();
    const bool achromatic = start.hsvHueF() < 0.0 || start.hsvSaturationF() < 0.05 || start.valueF() < 0.05;
    const double startHue = achromatic ? 0.0 : start.hsvHueF() * 360.0;
    const double saturation = achromatic ? 1.0 : start.hsvSaturationF();
    const double value = achromatic ? 1.0 : start.valueF();

    double hue = std::fmod(startHue + dx * kDegreesPerPixel, 360.0);
    if(hue < 0.0)
        hue += 360.0;

    setColor(QColor::fromHsvF(float(hue / 360.0), float(saturation), float(value), start.alphaF()));
}

void ColorWheelSwatch::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !m_impl->pressed)
        return;
    m_impl->pressed = false;

    if(m_impl->dragging)
    {
        m_impl->dragging = false;
        unsetCursor();
        emit finishedEditing();
        return;
    }

    // A plain click: the full picker, as before.
    emit beganEditing();
    emit swatchClicked();
    m_impl->openPicker(this);
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

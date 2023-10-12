#include <QPainter>
#include <QResizeEvent>
#include "gradientbar.h"
#include "vektor/vektorartist.h"

namespace exo {

GradientHandle::GradientHandle()
{
    handlePath.moveTo(0,0);
    handlePath.lineTo(+5,5);
    handlePath.lineTo(+5,15);
    handlePath.lineTo(-5,15);
    handlePath.lineTo(-5,5);
    handlePath.closeSubpath();
}

GradientHandle::GradientHandle(Gradient::StopData stop, unsigned index):color(stop.color),position(stop.position),midPosition(stop.midPosition),index(index)
{
    handlePath.moveTo(0,0);
    handlePath.lineTo(+5,5);
    handlePath.lineTo(+5,15);
    handlePath.lineTo(-5,15);
    handlePath.lineTo(-5,5);
    handlePath.closeSubpath();
}

void GradientHandle::reposition(int gradientWidth)
{
    handlePosition = QPoint{static_cast<int>(position * gradientWidth),40};
}

bool GradientHandle::hitTest(const QPoint &pt)
{
    isHovering = handlePath.contains(pt);
    return isHovering;
}

void GradientHandle::paint(QPainter *painter)
{
    if(isSelected)
        painter->setPen(Qt::red);
    else
        painter->setPen(isHovering ? Qt::cyan : Qt::white);
    painter->setBrush(color.toQColor());
    painter->drawPath(handlePath);
}

GradientMidPoint::GradientMidPoint()
{
    handlePath.addEllipse(-3,2,6,6);
}

bool GradientMidPoint::hitTest(const QPoint &pt)
{
    isHovering = handlePath.contains(pt);
    return isHovering;
}

void GradientMidPoint::paint(QPainter *painter)
{
    if(!isVisible)
        return;
    painter->setPen(isHovering ? Qt::cyan : Qt::black);
    painter->drawPath(handlePath);
}

GradientBar::GradientBar(const Gradient &gradient, QWidget *parent) : QWidget(parent),m_gradient(gradient)
{
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


    //for(const Gradient::StopData &stop : gradient.stops)
        //m_handles.append(GradientHandle{stop, static_cast<uint>(m_handles.size())});
    //updateHandles();
    setMouseTracking(true);

    VektorArtist artist(&m_gradientBarVektor);
    artist.drawRect(0,0,100,40,1);
    artist.endVektor();

    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}

void GradientBar::rebuildGradientFromHandles()
{
    m_gradient.stops.clear();
    for(GradientHandle &handle : m_handles)
    {
        if(!handle.isVisible)
            continue;
        handle.index = m_gradient.addColor(handle.color, handle.position, handle.midPosition);
    }
}

void GradientBar::updateHandles()
{
    int counter = 0;
    m_handles.resize(static_cast<int>(m_gradient.size()));
    m_midpointHandles.resize(static_cast<int>(m_gradient.size()));

    double lastX = 0;

    for(const Gradient::StopData &stop : m_gradient.stops)
    {
        GradientHandle &handle = m_handles[counter];
        handle.color = stop.color;
        handle.position = stop.position;
        handle.midPosition = stop.midPosition;
        handle.reposition(m_gradientImage.width());
        handle.index = static_cast<uint>(counter);
        handle.isVisible = true;

        GradientMidPoint &midPointHandle = m_midpointHandles[counter];
        midPointHandle.index = handle.index;
        midPointHandle.midPosition = handle.midPosition;
        midPointHandle.handlePosition = QPoint(((handle.handlePosition.x() - lastX) * handle.midPosition) + lastX, 40);
        midPointHandle.isVisible = true;

        lastX = handle.handlePosition.x();
        ++counter;
    }
    m_midpointHandles.front().isVisible = false;

    updateMidPointHandles();
}

void GradientBar::updateMidPointHandles()
{

    int counter = 0;
    double lastX = 0;
    for(const Gradient::StopData &stop : m_gradient.stops)
    {
        GradientHandle &handle = m_handles[counter];
        GradientMidPoint &midPointHandle = m_midpointHandles[counter];
        midPointHandle.handlePosition = QPoint(((handle.handlePosition.x() - lastX) * handle.midPosition) + lastX, 40);

        lastX = handle.handlePosition.x();
        ++counter;
    }
}

void GradientBar::setGradient(const Gradient &gradient)
{
    m_gradient = gradient;
    updateHandles();

    redrawGradient();
    update();
}

void GradientBar::redrawGradient()
{
    m_gradientImage = QImage{width() - (m_gradientInset * 2), 40,QImage::Format_ARGB32};
    m_gradientImage.fill(Qt::red);
    QPainter painter{&m_gradientImage};
    painter.fillRect(m_gradientImage.rect(), m_alphaBGBrush);

    Gradient g = m_gradient;
    g.type = Gradient::TypeLinear;
    g.overshoot = Gradient::OverShootClamp;
    g.angle = 0.0;

    rendering_buffer buffer(m_gradientImage.bits(), static_cast<uint>(m_gradientImage.width()),40,m_gradientImage.bytesPerLine());
    m_renderer.render(buffer,&m_gradientBarVektor,trans_affine{});
}

void GradientBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter{this};
    painter.drawImage(QPoint(m_gradientInset, 0),m_gradientImage);

    painter.translate(m_gradientInset,0);
    for(GradientHandle &handle : m_handles)
    {
        if(!handle.isVisible)
            continue;
        painter.translate(handle.handlePosition);
        handle.paint(&painter);
        painter.translate(-handle.handlePosition);
    }

    painter.setBrush(Qt::white);

    if(m_gradient.mode == Gradient::BlendBezier)
    {
        for(GradientMidPoint &midpointHandle : m_midpointHandles)
        {
            if(!midpointHandle.isVisible)
                continue;
            painter.translate(midpointHandle.handlePosition);
            midpointHandle.paint(&painter);
            painter.translate(-midpointHandle.handlePosition);
        }
    }

}

void GradientBar::resizeEvent(QResizeEvent *event)
{
    m_gradientBarVektor.clear();
    int gradientWidth = event->size().width() - (m_gradientInset * 2);
    //m_gradientBarVektor.setFillTransform(gradient_affine(0, 0, gradientWidth, 0, 1).inverted());


    VektorArtist artist(&m_gradientBarVektor);
    artist.drawRect(0,0,gradientWidth,40,1);
    artist.endVektor();
    redrawGradient();

    updateHandles();
}

void GradientBar::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseMoveEvent(event);
    QPoint pt = event->pos() - QPoint(m_gradientInset,0);

    int gradientWidth = m_gradientImage.width();

    if(m_activeMidPoint && event->buttons() & Qt::LeftButton)
    {
        double left = 0;
        double right = 1;
        if(m_activeMidPoint->index > 0)
            left = m_handles[m_activeMidPoint->index-1].handlePosition.x();
        if(m_activeMidPoint->index < m_handles.size())
            right = m_handles[m_activeMidPoint->index].handlePosition.x();

        m_handles[m_activeMidPoint->index].midPosition = std::min(.95,std::max(0.05,(pt.x() - left) / (right - left)));

        rebuildGradientFromHandles();
        updateMidPointHandles();
        emit gradientChanged(m_gradient);

        redrawGradient();
        update();
        return;
    }

    if(m_activeHandle && event->buttons() & Qt::LeftButton)
    {
        m_activeHandle->position = std::min(1.0,std::max(0.0,pt.x()/static_cast<double>(gradientWidth)));
        m_activeHandle->reposition(gradientWidth);
        m_activeHandle->isVisible = (pt.y() < 65 && pt.y() > 0) || m_handles.size() == 2;

        rebuildGradientFromHandles();
        updateMidPointHandles();
        emit gradientChanged(m_gradient);

        redrawGradient();
    }
    else {
        for(GradientHandle &handle : m_handles)
        {
            handle.hitTest(pt - handle.handlePosition);
        }

        for(GradientMidPoint &midpointHandle : m_midpointHandles)
        {
            midpointHandle.hitTest(pt - midpointHandle.handlePosition);
        }
    }

    update();

}

void GradientBar::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mousePressEvent(event);

    emit beginEditing();
    m_activeMidPoint = nullptr;
    QPoint pt = event->pos() - QPoint(m_gradientInset,0);

    for(GradientMidPoint &midpointHandle : m_midpointHandles)
    {
        if(midpointHandle.isVisible && midpointHandle.hitTest(pt - midpointHandle.handlePosition))
        {
            m_activeMidPoint = &midpointHandle;
            return;
        }
    }

    for(GradientHandle &handle : m_handles)
    {
        if(handle.hitTest(pt - handle.handlePosition))
        {
            if(m_activeHandle)
            {
                if(m_activeHandle == &handle)
                    return;
                m_activeHandle->isSelected = false;
            }

            m_activeHandle = &handle;
            m_activeHandle->isSelected = true;
            updateMidPointHandles();
            emit stopSelected(m_activeHandle->index);
            return;
        }
    }

    if(m_activeHandle && !m_activeMidPoint)
    {
        m_activeHandle->isSelected = false;
        m_activeHandle = nullptr;
        emit stopDeselected();
    }

    if(m_gradientImage.rect().contains(event->pos()))
    {
        double position = pt.x()/static_cast<double>(m_gradientImage.width());
        auto addedStop = m_gradient.addColor(m_gradient.colorAtPosition(position),position);
        updateHandles();

        emit gradientChanged(m_gradient);
        emit stopSelected(addedStop);
        update();
    }
}

void GradientBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mouseReleaseEvent(event);
    if(m_activeHandle)
    {
        //m_activeHandle->isSelected = false;
        //emit stopSelected(m_activeHandle->index);

        if(!m_activeHandle->isVisible)
        {
            for(auto it = m_handles.begin(); it != m_handles.end(); ++it)
            {
                if((*it).index == m_activeHandle->index)
                {
                    m_handles.erase(it);
                    updateHandles();

                    break;
                }
            }
        }
    }
    Q_UNUSED(event)
    update();
    emit endEditing();
}


} // namespace exo

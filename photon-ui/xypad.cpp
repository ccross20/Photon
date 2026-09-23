#include "xypad.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <limits>

namespace photon {

static const int kInset = 4;   // handle keep-in margin

XYPad::XYPad(QWidget *parent) : QWidget(parent),
    m_minimum(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()),
    m_maximum(std::numeric_limits<double>::max(), std::numeric_limits<double>::max())
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setMinimumSize(40, 40);
}

bool XYPad::isBounded() const
{
    return m_minimum.x() > -1e18 && m_maximum.x() < 1e18 && m_maximum.x() > m_minimum.x()
        && m_minimum.y() > -1e18 && m_maximum.y() < 1e18 && m_maximum.y() > m_minimum.y();
}

QPointF XYPad::clamp(const QPointF &p) const
{
    return QPointF(qBound(m_minimum.x(), p.x(), m_maximum.x()),
                   qBound(m_minimum.y(), p.y(), m_maximum.y()));
}

void XYPad::setRange(const QPointF &t_min, const QPointF &t_max)
{
    m_minimum = t_min;
    m_maximum = t_max;
    m_value = clamp(m_value);
    update();
}

void XYPad::setValue(const QPointF &t_value)
{
    if(m_scrubbing)
        return;   // don't fight an in-progress drag
    const QPointF v = clamp(t_value);
    if(v == m_value)
        return;
    m_value = v;
    update();
}

QPointF XYPad::valueForPos(const QPoint &pos) const
{
    const QRectF r = QRectF(rect()).adjusted(kInset, kInset, -kInset, -kInset);
    const double fx = qBound(0.0, (pos.x() - r.left()) / r.width(), 1.0);
    const double fy = qBound(0.0, (pos.y() - r.top()) / r.height(), 1.0);
    return QPointF(m_minimum.x() + fx * (m_maximum.x() - m_minimum.x()),
                   m_minimum.y() + fy * (m_maximum.y() - m_minimum.y()));
}

void XYPad::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }
    m_pressed = true;
    m_pressGlobal = event->globalPosition().toPoint();
    m_lastGlobal = m_pressGlobal;
    m_dragOffset = QPoint(0, 0);

    if(isBounded())
    {
        // Absolute: jump straight to the clicked position.
        m_scrubbing = true;
        m_value = valueForPos(event->position().toPoint());
        emit valueChanged(m_value);
        update();
    }
    else
    {
        // Relative: wait for movement before scrubbing.
        m_scrubbing = false;
        m_scrubAccum = m_value;
    }
    event->accept();
}

void XYPad::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_pressed)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if(isBounded())
    {
        m_value = valueForPos(event->position().toPoint());
        emit valueChanged(m_value);
        update();
        event->accept();
        return;
    }

    const QPoint g = event->globalPosition().toPoint();

    if(!m_scrubbing)
    {
        if((g - m_pressGlobal).manhattanLength() <= 3)
            return;
        m_scrubbing = true;
        setCursor(Qt::BlankCursor);
    }

    double perPixel = 0.01;
    if(event->modifiers() & Qt::ShiftModifier)
        perPixel *= 0.1;
    else if(event->modifiers() & Qt::ControlModifier)
        perPixel *= 10.0;

    const QPoint d = g - m_lastGlobal;
    m_lastGlobal = g;
    m_scrubAccum += QPointF(d.x() * perPixel, d.y() * perPixel);
    m_value = clamp(m_scrubAccum);
    m_dragOffset = g - m_pressGlobal;
    emit valueChanged(m_value);
    update();

    // Wrap the hidden cursor at the screen edges for infinite drag.
    if(QScreen *screen = QGuiApplication::screenAt(g))
    {
        const QRect avail = screen->geometry();
        QPoint np = g;
        if(g.x() <= avail.left() + 2)       np.setX(avail.right() - 3);
        else if(g.x() >= avail.right() - 2) np.setX(avail.left() + 3);
        if(g.y() <= avail.top() + 2)        np.setY(avail.bottom() - 3);
        else if(g.y() >= avail.bottom() - 2)np.setY(avail.top() + 3);
        if(np != g)
        {
            QCursor::setPos(np);
            m_lastGlobal = np;
        }
    }

    event->accept();
}

void XYPad::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !m_pressed)
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    m_pressed = false;

    const bool wasScrubbing = m_scrubbing;
    m_scrubbing = false;

    if(wasScrubbing && !isBounded())
    {
        QCursor::setPos(m_pressGlobal);
        setCursor(m_hovered ? Qt::SizeAllCursor : Qt::ArrowCursor);
        m_dragOffset = QPoint(0, 0);
    }
    emit editingFinished();
    update();
    event->accept();
}

void XYPad::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    if(!m_scrubbing)
        setCursor(isBounded() ? Qt::CrossCursor : Qt::SizeAllCursor);
    update();
    QWidget::enterEvent(event);
}

void XYPad::leaveEvent(QEvent *event)
{
    m_hovered = false;
    if(!m_scrubbing)
        unsetCursor();
    update();
    QWidget::leaveEvent(event);
}

void XYPad::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const double radius = 4.0;

    QPainterPath clip;
    clip.addRoundedRect(r, radius, radius);
    p.setClipPath(clip);
    p.fillRect(r, m_hovered ? QColor(48, 48, 52) : QColor(40, 40, 44));

    // Faint centre crosshair.
    p.setPen(QColor(255, 255, 255, 22));
    p.drawLine(QPointF(r.center().x(), r.top()), QPointF(r.center().x(), r.bottom()));
    p.drawLine(QPointF(r.left(), r.center().y()), QPointF(r.right(), r.center().y()));

    const QRectF inner = r.adjusted(kInset, kInset, -kInset, -kInset);

    QPointF handle;
    if(isBounded())
    {
        const double fx = qBound(0.0, (m_value.x() - m_minimum.x()) / (m_maximum.x() - m_minimum.x()), 1.0);
        const double fy = qBound(0.0, (m_value.y() - m_minimum.y()) / (m_maximum.y() - m_minimum.y()), 1.0);
        handle = QPointF(inner.left() + fx * inner.width(), inner.top() + fy * inner.height());

        // Guide lines to the handle.
        p.setPen(QColor(64, 108, 160, 120));
        p.drawLine(QPointF(inner.left(), handle.y()), QPointF(handle.x(), handle.y()));
        p.drawLine(QPointF(handle.x(), inner.top()), QPointF(handle.x(), handle.y()));
    }
    else
    {
        // Relative: dot sits at centre, nudged toward the current drag direction.
        QPointF off(m_dragOffset);
        const double maxOff = inner.width() * 0.5;
        const double len = std::hypot(off.x(), off.y());
        if(len > maxOff && len > 0.0)
            off *= maxOff / len;
        handle = r.center() + off;
    }

    p.setClipping(false);

    p.setPen(QPen(m_hovered ? QColor(96, 96, 104) : QColor(64, 64, 68), 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, radius, radius);

    // Handle.
    p.setPen(Qt::NoPen);
    p.setBrush(m_scrubbing ? QColor(90, 150, 220) : QColor(70, 120, 180));
    p.drawEllipse(handle, 4.0, 4.0);
    p.setBrush(QColor(230, 230, 230));
    p.drawEllipse(handle, 1.5, 1.5);
}

} // namespace photon

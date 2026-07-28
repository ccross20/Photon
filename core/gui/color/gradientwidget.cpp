#include "gradientwidget.h"
#include "colorselectordialog.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLinearGradient>

namespace photon {

static const int kInset = 8;        // horizontal margin so end handles stay in view
static const int kHandleH = 12;     // handle strip height
static const int kHalfHandle = 6;   // handle half-width

GradientWidget::GradientWidget(const Gradient &gradient, QWidget *parent)
    : QWidget(parent), m_gradient(gradient)
{
    setMinimumSize(140, 46);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(false);
}

void GradientWidget::setGradient(const Gradient &gradient)
{
    if(m_gradient == gradient)
        return;
    m_gradient = gradient;
    if(m_selected >= m_gradient.count())
        m_selected = -1;
    update();
}

QRect GradientWidget::barRect() const
{
    return QRect(kInset, 2, width() - 2 * kInset, height() - kHandleH - 4);
}

qreal GradientWidget::positionForX(int x) const
{
    const QRect r = barRect();
    if(r.width() <= 0)
        return 0.0;
    return qBound(0.0, double(x - r.left()) / double(r.width()), 1.0);
}

int GradientWidget::xForPosition(qreal position) const
{
    const QRect r = barRect();
    return r.left() + qRound(position * r.width());
}

int GradientWidget::handleAt(const QPoint &pos) const
{
    const int y = barRect().bottom();
    // Prefer the selected stop when handles overlap, then nearest.
    int best = -1;
    int bestDist = kHalfHandle + 3;
    for(int i = 0; i < m_gradient.count(); ++i)
    {
        const int hx = xForPosition(m_gradient.stopAt(i).position);
        if(pos.y() < y || pos.y() > y + kHandleH + 2)
            continue;
        const int d = qAbs(pos.x() - hx);
        if(d < bestDist || (d == bestDist && i == m_selected))
        {
            bestDist = d;
            best = i;
        }
    }
    return best;
}

void GradientWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRect bar = barRect();

    // Checkerboard behind the bar so alpha reads clearly.
    const int cs = 6;
    for(int y = 0; y < bar.height(); y += cs)
        for(int x = 0; x < bar.width(); x += cs)
        {
            const bool alt = ((x / cs) + (y / cs)) & 1;
            p.fillRect(QRect(bar.left() + x, bar.top() + y,
                             qMin(cs, bar.width() - x), qMin(cs, bar.height() - y)),
                       alt ? QColor(120, 120, 120) : QColor(90, 90, 90));
        }

    // Gradient fill.
    QLinearGradient lg(bar.left(), 0, bar.right(), 0);
    for(int i = 0; i < m_gradient.count(); ++i)
        lg.setColorAt(qBound(0.0, m_gradient.stopAt(i).position, 1.0), m_gradient.stopAt(i).color);
    p.fillRect(bar, lg);

    p.setPen(QColor(30, 30, 30));
    p.setBrush(Qt::NoBrush);
    p.drawRect(bar.adjusted(0, 0, -1, -1));

    // Handles.
    const int y = bar.bottom() + 1;
    for(int i = 0; i < m_gradient.count(); ++i)
    {
        const int hx = xForPosition(m_gradient.stopAt(i).position);
        QPainterPath path;
        path.moveTo(hx, y);
        path.lineTo(hx - kHalfHandle, y + kHandleH);
        path.lineTo(hx + kHalfHandle, y + kHandleH);
        path.closeSubpath();

        p.setBrush(m_gradient.stopAt(i).color);
        p.setPen(QPen(i == m_selected ? QColor(Qt::white) : QColor(20, 20, 20),
                      i == m_selected ? 2 : 1));
        p.drawPath(path);
    }
}

void GradientWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    const int hit = handleAt(event->pos());

    if(event->button() == Qt::RightButton)
    {
        if(hit >= 0)
        {
            m_selected = hit;
            removeSelected();
        }
        return;
    }

    if(event->button() != Qt::LeftButton)
        return;

    if(hit >= 0)
    {
        m_selected = hit;
        m_dragging = true;
        update();
    }
    else
    {
        m_selected = -1;
        update();
    }
}

void GradientWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_dragging || m_selected < 0)
        return;

    const qreal pos = positionForX(event->pos().x());
    m_selected = m_gradient.setStopPosition(m_selected, pos);
    emit gradientChanged(m_gradient);
    update();
}

void GradientWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_dragging = false;
}

void GradientWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return;

    const int hit = handleAt(event->pos());
    if(hit >= 0)
    {
        m_selected = hit;
        editStopColor(hit);
        return;
    }

    // Add a stop where the bar was clicked, seeded with the colour already there.
    if(barRect().contains(event->pos()) ||
       (event->pos().y() >= barRect().top() && event->pos().y() <= barRect().bottom() + kHandleH))
    {
        const qreal pos = positionForX(event->pos().x());
        m_selected = m_gradient.addStop(pos, m_gradient.colorAt(pos));
        emit gradientChanged(m_gradient);
        update();
    }
}

void GradientWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        removeSelected();
        return;
    }
    QWidget::keyPressEvent(event);
}

void GradientWidget::editStopColor(int index)
{
    if(index < 0 || index >= m_gradient.count())
        return;

    ColorSelectorDialog *dialog = new ColorSelectorDialog(m_gradient.stopAt(index).color, window());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();

    connect(dialog, &ColorSelectorDialog::selectionChanged, this, [this, index](QColor color){
        if(index < 0 || index >= m_gradient.count())
            return;
        m_gradient.setStopColor(index, color);
        emit gradientChanged(m_gradient);
        update();
    });
}

void GradientWidget::removeSelected()
{
    // Keep at least two stops so the gradient stays meaningful.
    if(m_selected < 0 || m_gradient.count() <= 2)
        return;

    m_gradient.removeStop(m_selected);
    m_selected = -1;
    emit gradientChanged(m_gradient);
    update();
}

} // namespace photon

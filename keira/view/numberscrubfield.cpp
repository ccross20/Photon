#include "numberscrubfield.h"

#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QtMath>
#include <limits>

namespace keira {

static QString trimZeros(QString s)
{
    if(s.contains('.'))
    {
        while(s.endsWith('0'))
            s.chop(1);
        if(s.endsWith('.'))
            s.chop(1);
    }
    return s;
}

NumberScrubField::NumberScrubField(QWidget *parent) : QWidget(parent),
    m_minimum(std::numeric_limits<double>::lowest()),
    m_maximum(std::numeric_limits<double>::max())
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    setMinimumWidth(60);

    m_edit = new QLineEdit(this);
    m_edit->setFrame(false);
    m_edit->setAlignment(Qt::AlignCenter);
    m_edit->setStyleSheet("QLineEdit { background: #202024; color: #eaeaea; border: 1px solid #4a86c8;"
                          " border-radius: 4px; selection-background-color: #4a86c8; }");
    m_edit->hide();
    m_edit->installEventFilter(this);
    connect(m_edit, &QLineEdit::returnPressed, this, [this](){ commitEdit(); });
}

NumberScrubField::~NumberScrubField() = default;

void NumberScrubField::setIsInteger(bool t_isInteger)
{
    m_isInteger = t_isInteger;
    if(m_isInteger)
        m_value = qRound(m_value);
    update();
}

void NumberScrubField::setRange(double t_min, double t_max)
{
    m_minimum = t_min;
    m_maximum = t_max;
    m_value = clamp(m_value);
    update();
}

void NumberScrubField::setDecimals(int t_decimals)
{
    m_decimals = qMax(0, t_decimals);
    update();
}

void NumberScrubField::setReadOnly(bool t_readOnly)
{
    m_readOnly = t_readOnly;
    update();
}

double NumberScrubField::value() const
{
    return m_isInteger ? qRound(m_value) : m_value;
}

void NumberScrubField::setValue(double t_value)
{
    // Don't fight an in-progress scrub or an open editor.
    if(m_scrubbing || (m_edit && m_edit->isVisible()))
        return;

    const double v = clamp(m_isInteger ? qRound(t_value) : t_value);
    if(qFuzzyCompare(v + 1.0, m_value + 1.0))
        return;
    m_value = v;
    update();
}

QSize NumberScrubField::sizeHint() const
{
    return QSize(90, 22);
}

bool NumberScrubField::isBounded() const
{
    return m_minimum > -1e18 && m_maximum < 1e18 && m_maximum > m_minimum;
}

double NumberScrubField::clamp(double v) const
{
    if(v < m_minimum) v = m_minimum;
    if(v > m_maximum) v = m_maximum;
    return v;
}

double NumberScrubField::step() const
{
    if(m_isInteger)
        return 1.0;
    if(isBounded())
        return (m_maximum - m_minimum) / 100.0;
    return 0.01;
}

QString NumberScrubField::formatted(double v) const
{
    if(m_isInteger)
        return QString::number(qRound(v));
    return trimZeros(QString::number(v, 'f', m_decimals));
}

// ---- editing ---------------------------------------------------------------

void NumberScrubField::beginEdit()
{
    if(m_readOnly)
        return;
    m_edit->setGeometry(rect());
    m_edit->setText(formatted(m_value));
    m_edit->show();
    m_edit->raise();
    m_edit->setFocus(Qt::MouseFocusReason);
    m_edit->selectAll();
    update();
}

void NumberScrubField::commitEdit()
{
    if(!m_edit->isVisible())
        return;

    bool ok = false;
    const double parsed = m_edit->text().toDouble(&ok);
    m_edit->hide();

    if(ok)
    {
        const double v = clamp(m_isInteger ? qRound(parsed) : parsed);
        m_value = v;
        emit valueChanged(m_value);
        emit editingFinished();
    }
    setFocus(Qt::OtherFocusReason);
    update();
}

void NumberScrubField::cancelEdit()
{
    m_edit->hide();
    setFocus(Qt::OtherFocusReason);
    update();
}

bool NumberScrubField::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_edit)
    {
        if(event->type() == QEvent::KeyPress)
        {
            auto *ke = static_cast<QKeyEvent *>(event);
            if(ke->key() == Qt::Key_Escape)
            {
                cancelEdit();
                return true;
            }
        }
        else if(event->type() == QEvent::FocusOut)
        {
            commitEdit();
        }
    }
    return QWidget::eventFilter(watched, event);
}

// ---- scrubbing -------------------------------------------------------------

void NumberScrubField::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || m_readOnly)
    {
        QWidget::mousePressEvent(event);
        return;
    }
    m_pressed = true;
    m_scrubbing = false;
    m_pressGlobal = event->globalPosition().toPoint();
    m_lastGlobal = m_pressGlobal;
    m_scrubAccum = m_value;
    event->accept();
}

void NumberScrubField::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_pressed)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint g = event->globalPosition().toPoint();

    if(!m_scrubbing)
    {
        if(qAbs(g.x() - m_pressGlobal.x()) <= 3)
            return;
        m_scrubbing = true;
        setCursor(Qt::BlankCursor);
    }

    // Per-pixel sensitivity: bounded params sweep their whole range across the
    // field width; unbounded ones use a fixed step.
    double perPixel = isBounded() ? (m_maximum - m_minimum) / qMax(1, width())
                                   : (m_isInteger ? 0.1 : 0.01);
    if(event->modifiers() & Qt::ShiftModifier)
        perPixel *= 0.1;      // fine
    else if(event->modifiers() & Qt::ControlModifier)
        perPixel *= 10.0;     // coarse

    const int dx = g.x() - m_lastGlobal.x();
    m_lastGlobal = g;
    m_scrubAccum += dx * perPixel;

    const double nv = clamp(m_isInteger ? qRound(m_scrubAccum) : m_scrubAccum);
    if(!qFuzzyCompare(nv + 1.0, m_value + 1.0))
    {
        m_value = nv;
        emit valueChanged(m_value);
        update();
    }

    // Wrap the (hidden) cursor at the screen edges so scrubbing never runs out
    // of room — the classic infinite-drag behaviour.
    if(QScreen *screen = QGuiApplication::screenAt(g))
    {
        const QRect avail = screen->geometry();
        if(g.x() <= avail.left() + 2)
        {
            const QPoint np(avail.right() - 3, g.y());
            QCursor::setPos(np);
            m_lastGlobal = np;
        }
        else if(g.x() >= avail.right() - 2)
        {
            const QPoint np(avail.left() + 3, g.y());
            QCursor::setPos(np);
            m_lastGlobal = np;
        }
    }

    event->accept();
}

void NumberScrubField::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton || !m_pressed)
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    m_pressed = false;

    if(m_scrubbing)
    {
        m_scrubbing = false;
        QCursor::setPos(m_pressGlobal);   // return the cursor to where the drag began
        setCursor(m_hovered ? Qt::SizeHorCursor : Qt::ArrowCursor);
        emit editingFinished();
    }
    else
    {
        beginEdit();   // a click without a drag: type a value
    }
    update();
    event->accept();
}

void NumberScrubField::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && !m_readOnly)
    {
        beginEdit();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void NumberScrubField::wheelEvent(QWheelEvent *event)
{
    if(m_readOnly)
        return;

    double s = step();
    if(event->modifiers() & Qt::ShiftModifier)
        s *= 0.1;
    else if(event->modifiers() & Qt::ControlModifier)
        s *= 10.0;

    const int dir = event->angleDelta().y() > 0 ? 1 : -1;
    const double nv = clamp(m_isInteger ? qRound(m_value + dir * qMax(1.0, s))
                                        : m_value + dir * s);
    if(!qFuzzyCompare(nv + 1.0, m_value + 1.0))
    {
        m_value = nv;
        emit valueChanged(m_value);
        emit editingFinished();
        update();
    }
    event->accept();
}

void NumberScrubField::keyPressEvent(QKeyEvent *event)
{
    if(m_readOnly)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    switch(event->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_Left:
    {
        const int dir = (event->key() == Qt::Key_Up || event->key() == Qt::Key_Right) ? 1 : -1;
        double s = step();
        if(event->modifiers() & Qt::ShiftModifier) s *= 0.1;
        else if(event->modifiers() & Qt::ControlModifier) s *= 10.0;
        const double nv = clamp(m_isInteger ? qRound(m_value + dir * qMax(1.0, s))
                                            : m_value + dir * s);
        if(!qFuzzyCompare(nv + 1.0, m_value + 1.0))
        {
            m_value = nv;
            emit valueChanged(m_value);
            emit editingFinished();
            update();
        }
        event->accept();
        return;
    }
    case Qt::Key_Return:
    case Qt::Key_Enter:
        beginEdit();
        event->accept();
        return;
    default:
        break;
    }

    // Start typing directly on a digit / sign / dot.
    const QString t = event->text();
    if(!t.isEmpty() && (t[0].isDigit() || t[0] == '-' || t[0] == '+' || t[0] == '.'))
    {
        beginEdit();
        m_edit->setText(t);
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void NumberScrubField::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    if(!m_readOnly && !m_scrubbing)
        setCursor(Qt::SizeHorCursor);
    update();
    QWidget::enterEvent(event);
}

void NumberScrubField::leaveEvent(QEvent *event)
{
    m_hovered = false;
    if(!m_scrubbing)
        unsetCursor();
    update();
    QWidget::leaveEvent(event);
}

// ---- painting --------------------------------------------------------------

void NumberScrubField::paintEvent(QPaintEvent *)
{
    if(m_edit->isVisible())
        return;   // the line editor covers us while typing

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const double radius = 4.0;

    QPainterPath clip;
    clip.addRoundedRect(r, radius, radius);
    p.setClipPath(clip);

    // Background.
    p.fillRect(r, m_hovered ? QColor(48, 48, 52) : QColor(40, 40, 44));

    // Range fill.
    if(isBounded())
    {
        const double frac = qBound(0.0, (m_value - m_minimum) / (m_maximum - m_minimum), 1.0);
        if(frac > 0.0)
        {
            QRectF fill = r;
            fill.setWidth(r.width() * frac);
            p.fillRect(fill, m_readOnly ? QColor(80, 80, 86) : QColor(64, 108, 160));
        }
    }

    p.setClipping(false);

    // Border.
    p.setPen(QPen(m_hovered ? QColor(96, 96, 104) : QColor(64, 64, 68), 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, radius, radius);

    // Value text.
    p.setPen(m_readOnly ? QColor(150, 150, 150) : QColor(230, 230, 230));
    p.drawText(rect(), Qt::AlignCenter, formatted(m_value));
}

} // namespace keira

#ifndef PHOTON_XYPAD_H
#define PHOTON_XYPAD_H

#include <QWidget>
#include <QPointF>
#include <QPoint>
#include "photon-global.h"

namespace photon {

// A small 2D control that drives both axes of a point at once. With a finite
// range it acts as an absolute XY pad (the handle shows where the value sits);
// unbounded, it acts as a relative scrubber (drag to nudge both axes, Shift =
// fine, Ctrl = coarse), mirroring NumberScrubField's feel.
class PHOTONCORE_EXPORT XYPad : public QWidget
{
    Q_OBJECT
public:
    explicit XYPad(QWidget *parent = nullptr);

    QPointF value() const { return m_value; }
    void setRange(const QPointF &minimum, const QPointF &maximum);

    QSize sizeHint() const override { return QSize(48, 48); }

public slots:
    void setValue(const QPointF &);

signals:
    void valueChanged(QPointF);
    void editingFinished();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    bool isBounded() const;
    QPointF clamp(const QPointF &) const;
    QPointF valueForPos(const QPoint &) const;   // absolute mode

    QPointF m_value{0.0, 0.0};
    QPointF m_minimum;
    QPointF m_maximum;

    bool m_pressed = false;
    bool m_scrubbing = false;
    bool m_hovered = false;
    QPoint m_pressGlobal;
    QPoint m_lastGlobal;
    QPointF m_scrubAccum;
    QPoint m_dragOffset;   // relative-mode visual feedback
};

} // namespace photon

#endif // PHOTON_XYPAD_H

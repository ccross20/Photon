#ifndef PHOTON_GRADIENTWIDGET_H
#define PHOTON_GRADIENTWIDGET_H

#include <QWidget>
#include "photon-global.h"
#include "util/gradient.h"

namespace photon {

// Inline editor for a Gradient: a preview bar with draggable colour stops.
//   - drag a handle to move a stop
//   - double-click a handle to edit its colour
//   - double-click the bar to add a stop
//   - select a handle and press Delete (or right-click it) to remove it
class PHOTONCORE_EXPORT GradientWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GradientWidget(const Gradient &gradient = Gradient{}, QWidget *parent = nullptr);

    const Gradient &gradient() const { return m_gradient; }

public slots:
    void setGradient(const Gradient &gradient);

signals:
    void gradientChanged(const Gradient &gradient);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    QRect barRect() const;
    int handleAt(const QPoint &pos) const;    // -1 if none
    qreal positionForX(int x) const;
    int xForPosition(qreal position) const;
    void editStopColor(int index);
    void removeSelected();

    Gradient m_gradient;
    int m_selected = -1;
    bool m_dragging = false;
};

} // namespace photon

#endif // PHOTON_GRADIENTWIDGET_H

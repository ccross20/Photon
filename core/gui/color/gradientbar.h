#ifndef GRADIENTBAR_H
#define GRADIENTBAR_H


#include <QWidget>
#include <QPainterPath>

#include "exo-core_global.h"
#include "vektor/gradient.h"
#include "vektor/vektorrenderer.h"

namespace exo {

class EXOCORE_EXPORT GradientHandle
{
public:
    GradientHandle();
    GradientHandle(Gradient::StopData stop, unsigned index);
    bool hitTest(const QPoint &pt);
    void paint(QPainter *painter);
    void reposition(int gradientWidth);

    QPainterPath handlePath;
    Color color;
    QPoint handlePosition;
    double position;
    double midPosition;
    unsigned index;
    bool isHovering = false;
    bool isSelected = false;
    bool isVisible = true;
};

class EXOCORE_EXPORT GradientMidPoint
{
public:
    GradientMidPoint();
    bool hitTest(const QPoint &pt);
    void paint(QPainter *painter);

    QPainterPath handlePath;
    QPoint handlePosition;
    double midPosition;
    unsigned index;
    bool isHovering = false;
    bool isVisible = true;
};

class EXOCORE_EXPORT GradientBar : public QWidget
{
    Q_OBJECT
public:

    explicit GradientBar(const Gradient &gradient, QWidget *parent = nullptr);

signals:
    void gradientChanged(Gradient gradient);
    void stopSelected(unsigned stopIndex);
    void stopDeselected();
    void beginEditing();
    void endEditing();

public slots:
    void setGradient(const Gradient &gradient);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void redrawGradient();
    void updateHandles();
    void updateMidPointHandles();
    void rebuildGradientFromHandles();

    QBrush m_alphaBGBrush;
    Gradient m_gradient;
    QImage m_gradientImage;
    VektorRenderer m_renderer;
    Vektor m_gradientBarVektor;
    QVector<GradientHandle> m_handles;
    QVector<GradientMidPoint> m_midpointHandles;
    GradientHandle *m_activeHandle = nullptr;
    GradientMidPoint *m_activeMidPoint = nullptr;
    int m_gradientInset = 5;
};

} // namespace exo

#endif // GRADIENTBAR_H

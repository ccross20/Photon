#ifndef COLORWHEEL_P_H
#define COLORWHEEL_P_H

#include <QWidget>

namespace exo {

class ColorWheel : public QWidget
{
    Q_OBJECT
public:

    enum InteractionType
    {
        InteractionNone,
        InteractionWheel,
        InteractionTriangle
    };

    explicit ColorWheel(QWidget *parent = nullptr);
    ~ColorWheel() override;

    QColor value() const;
    void setValue(const QColor &value);
signals:
    void colorChange(QColor color);
    void beginEditing();
    void endEditing();

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:

    void wheelPress(const QPointF &pt);
    void trianglePress(const QPointF &pt);
    void renderWheel();
    void renderTriangle();

    QImage *m_triangle;
    QPixmap m_wheel;
    QPointF m_center;
    QPolygonF m_trianglePoly;

    InteractionType m_interaction = InteractionNone;
    double m_radius = 0.0;
    double m_innerRadius = 0.0;
    double m_hue = 0.0;
    double m_saturation = 1.0;
    double m_lightness = 1.0;
    double m_shortX = 0.0;
    double m_triHeight = 0.0;
};

} // namespace exo

#endif // COLORWHEEL_P_H

#ifndef COLORSLIDER_H
#define COLORSLIDER_H
#include <QWidget>

namespace photon {

class ColorSlider : public QWidget
{
    Q_OBJECT
public:
    enum ColorMode{
        Mode_RGBA_Red,
        Mode_RGBA_Blue,
        Mode_RGBA_Green,
        Mode_RGBA_Alpha,
        Mode_HSVA_Hue,
        Mode_HSVA_Saturation,
        Mode_HSVA_Value,
        Mode_HSVA_Alpha,
        Mode_HSLA_Hue,
        Mode_HSLA_Saturation,
        Mode_HSLA_Lightness,
        Mode_HSLA_Alpha
    };
    ColorSlider(const QColor &color, ColorMode mode, double value = 0.0, QWidget *parent = nullptr);
    ColorSlider(ColorMode mode, double value, QWidget *parent = nullptr);

    double value() const;
    void setValue(const double &value);
    void setColors(const QColor &colorA, const QColor &colorB);
    void setColor(const QColor &color);
    void setTrackRadius(int value);
    int trackRadius() const;
    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

signals:
    void colorChange(const QColor &color);
    void valueChange(double value);
    void beginEditing();
    void endEditing();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void handleMouse(const QPoint &pt);
    QSize m_trackInset;
    QRect m_trackRect;
    int m_trackRadius = -1;
    int m_calculatedRadius = 0;
    QBrush m_alphaBGBrush;
    QColor m_colorA;
    QColor m_colorB;
    QColor m_colorC;
    double m_value = 0.0;
    ColorMode m_mode;
    Qt::Orientation m_orientation = Qt::Horizontal;
};

} // namespace exo

#endif // COLORSLIDER_H

#ifndef HUEBOX_H
#define HUEBOX_H
#include <QWidget>
#include "colorslider.h"

namespace photon {


class ColorBox : public QWidget
{
    Q_OBJECT
public:
    explicit ColorBox(QWidget *parent = nullptr);

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
    void renderBox();
    QImage m_gradientImage;
    QColor m_color;
};

class HueBox : public QWidget
{
    Q_OBJECT
public:
    explicit HueBox(QWidget *parent = nullptr);

    QColor value() const;
    void setValue(const QColor &value);
    void setShowAlpha(bool value);
    bool showAlpha() const;
signals:
    void beginEditing();
    void endEditing();
    void colorChange(QColor color);

public slots:
    void boxColorChange(QColor color);
    void hueSliderChange(double value);
    void alphaSliderChange(double value);


private:
    ColorBox *m_colorBox;
    ColorSlider *m_hueSlider;
    ColorSlider *m_alphaSlider;
    bool m_showAlpha = true;
};

} // namespace exo

#endif // HUEBOX_H

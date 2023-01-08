#ifndef COLORWHEELSWATCH_P_H
#define COLORWHEELSWATCH_P_H

#include "photon-global.h"
#include <QWidget>

namespace photon {

class PHOTONCORE_EXPORT ColorWheelSwatch : public QWidget
{
    Q_OBJECT
public:
    explicit ColorWheelSwatch(const QColor &color = QColor(Qt::black), QWidget *parent = nullptr);
    ~ColorWheelSwatch() override;
    const QColor &color() const;

signals:
    void swatchClicked();
    void wheelClicked();
    void colorChanged(QColor color);
    void beganEditing();
    void finishedEditing();

public slots:
    void setColor(const QColor &color);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
    Q_DISABLE_COPY(ColorWheelSwatch)

};

} // namespace photon

#endif // COLORWHEELSWATCH_P_H

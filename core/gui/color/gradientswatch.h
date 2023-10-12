#ifndef DECO_GRADIENTSWATCH_H
#define DECO_GRADIENTSWATCH_H

#include "exo-core_global.h"
#include "vektor/gradient.h"
#include <QWidget>

namespace exo {

class EXOCORE_EXPORT GradientSwatch : public QWidget
{
    Q_OBJECT
public:
    explicit GradientSwatch(const Gradient &gradient = Gradient{}, QWidget *parent = nullptr);
    ~GradientSwatch() override;
    const Gradient &gradient() const;

signals:
    void swatchClicked();
    void wheelClicked();
    void gradientChanged(const Gradient &gradient);
    void beganEditing();
    void finishedEditing();

public slots:
    void setGradient(const Gradient &gradient);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
    Q_DISABLE_COPY(GradientSwatch)

};

} // namespace exo

#endif // DECO_GRADIENTSWATCH_H

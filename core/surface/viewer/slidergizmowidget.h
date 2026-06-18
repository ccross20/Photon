#ifndef SLIDERGIZMOWIDGET_H
#define SLIDERGIZMOWIDGET_H

#include <QSlider>
#include "surfacegizmowidget.h"
#include "surface/slidergizmo.h"

namespace photon {


class CustomSlider : public QSlider
{
public:
    CustomSlider();

    void setText(const QString &);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QString m_text;

};


class SliderGizmoWidget : public SurfaceGizmoWidget
{
public:
    SliderGizmoWidget(SliderGizmo *gizmo, SurfaceMode mode);

    void updateGizmo() override;
private:
    CustomSlider *m_slider;
    SliderGizmo *m_gizmo;
};

} // namespace photon

#endif // SLIDERGIZMOWIDGET_H

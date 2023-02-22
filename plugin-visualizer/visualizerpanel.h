#ifndef PHOTON_VISUALIZERPANEL_H
#define PHOTON_VISUALIZERPANEL_H

#include "gui/panel.h"
#include "openglviewport.h"

namespace photon {

class FixtureModel;
class TransformComponent;
class MaterialComponent;

class VisualizerPanel : public Panel
{
    Q_OBJECT
public:
    VisualizerPanel();
    ~VisualizerPanel();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;

private slots:

    void dmxUpdated();

private:
    OpenGLViewport *m_viewport;
    QVector<FixtureModel*> m_models;
    TransformComponent *m_transform;
    MaterialComponent *m_material;
    int m_clickCounter = 0;
};

} // namespace photon

#endif // PHOTON_VISUALIZERPANEL_H

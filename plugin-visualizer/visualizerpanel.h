#ifndef PHOTON_VISUALIZERPANEL_H
#define PHOTON_VISUALIZERPANEL_H

#include "gui/panel.h"
#include "openglviewport.h"

namespace photon {

class TransformComponent;
class MaterialComponent;
class Scene;

class VisualizerPanel : public Panel
{
    Q_OBJECT
public:
    VisualizerPanel();
    ~VisualizerPanel();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;

private slots:


private:
    OpenGLViewport *m_viewport;
    TransformComponent *m_transform;
    MaterialComponent *m_material;
    Scene *m_scene;
    int m_clickCounter = 0;
};

} // namespace photon

#endif // PHOTON_VISUALIZERPANEL_H

#ifndef PHOTON_VISUALIZERPANEL_H
#define PHOTON_VISUALIZERPANEL_H

#include "gui/panel.h"
#include "openglviewport.h"

namespace photon {

class FixtureModel;

class VisualizerPanel : public Panel
{
    Q_OBJECT
public:
    VisualizerPanel();
    ~VisualizerPanel();

protected:

private slots:

    void dmxUpdated();

private:
    OpenGLViewport *m_viewport;
    QVector<FixtureModel*> m_models;
};

} // namespace photon

#endif // PHOTON_VISUALIZERPANEL_H

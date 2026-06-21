#ifndef PHOTON_VISUALIZERPANEL_H
#define PHOTON_VISUALIZERPANEL_H

#include "gui/panel.h"

namespace photon {

class RhiViewport;

class VisualizerPanel : public Panel
{
    Q_OBJECT
public:
    VisualizerPanel();
    ~VisualizerPanel();

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;

private slots:
    void tick();

private:
    RhiViewport *m_viewport;
};

} // namespace photon

#endif // PHOTON_VISUALIZERPANEL_H

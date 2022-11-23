#ifndef PHOTON_DMXVIEWERPANEL_H
#define PHOTON_DMXVIEWERPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class PHOTONCORE_EXPORT DMXViewerPanel : public Panel
{
    Q_OBJECT
public:
    DMXViewerPanel();
    ~DMXViewerPanel();

private slots:
    void tick();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DMXVIEWERPANEL_H

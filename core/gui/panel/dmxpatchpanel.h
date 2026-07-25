#ifndef PHOTON_DMXPATCHPANEL_H
#define PHOTON_DMXPATCHPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

// Hosts the DMXPatchGrid plus a universe selector.
class PHOTONCORE_EXPORT DMXPatchPanel : public Panel
{
public:
    DMXPatchPanel();
    ~DMXPatchPanel();

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DMXPATCHPANEL_H

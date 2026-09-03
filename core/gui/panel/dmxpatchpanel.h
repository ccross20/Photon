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

    // The grid sits in a QScrollArea with setWidgetResizable(false), so the
    // scroll area's own sizeHint() doesn't reflect the grid's real (fixed)
    // size - override so a floating window opens big enough to show the
    // whole channel grid without a manual resize first.
    QSize sizeHint() const override;

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DMXPATCHPANEL_H

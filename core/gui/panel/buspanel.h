#ifndef PHOTON_BUSPANEL_H
#define PHOTON_BUSPANEL_H
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class PHOTONCORE_EXPORT BusPanel : public Panel
{
public:
    BusPanel();
    ~BusPanel();

protected:
    void projectDidOpen(photon::Project* project) override;
    void projectWillClose(photon::Project* project) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_BUSPANEL_H

#ifndef PHOTON_ROUTINEEDITPANEL_H
#define PHOTON_ROUTINEEDITPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class PHOTONCORE_EXPORT RoutineEditPanel : public Panel
{
public:
    RoutineEditPanel();
    ~RoutineEditPanel();

    void setRoutine(Routine *);
    Routine *routine() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ROUTINEEDITPANEL_H

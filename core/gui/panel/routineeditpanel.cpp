#include <QVBoxLayout>
#include <QMenu>
#include "routineeditpanel.h"
#include "view/viewer.h"
#include "view/scene.h"
#include "routine/routine.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

class RoutineEditPanel::Impl
{
public:
    keira::Scene *scene;
    keira::Viewer *viewer;
};

RoutineEditPanel::RoutineEditPanel() : Panel("photon.routine"),m_impl(new Impl)
{
    m_impl->viewer = new keira::Viewer;
    setPanelWidget(m_impl->viewer);
    setName("Routine");

    m_impl->scene = new keira::Scene;
    //m_impl->scene->setIsAutoEvaluate(false);
    m_impl->scene->setNodeLibrary(photonApp->plugins()->nodeLibrary());

    m_impl->viewer->setScene(m_impl->scene);




}

RoutineEditPanel::~RoutineEditPanel()
{
    delete m_impl->scene;
    delete m_impl;
}

void RoutineEditPanel::setRoutine(Routine *t_routine)
{
    m_impl->scene->setGraph(t_routine);
    setName(t_routine->name());

}

Routine *RoutineEditPanel::routine() const
{
    return static_cast<Routine*>(m_impl->scene->graph());
}

} // namespace photon

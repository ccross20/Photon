#include <QVBoxLayout>
#include <QMenu>
#include "routineeditpanel.h"
#include "view/graphwidget.h"
#include "view/scene.h"
#include "routine/routine.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

class RoutineEditPanel::Impl
{
public:
    keira::Scene *scene;
    keira::GraphWidget *viewer;
};

RoutineEditPanel::RoutineEditPanel() : Panel("photon.routine"),m_impl(new Impl)
{
    m_impl->viewer = new keira::GraphWidget(photonApp->plugins()->nodeLibrary());
    setPanelWidget(m_impl->viewer);
    setName("Routine");

    m_impl->scene = new keira::Scene;
    //m_impl->scene->setIsAutoEvaluate(false);
    m_impl->scene->setNodeLibrary(photonApp->plugins()->nodeLibrary());

    m_impl->viewer->setScene(m_impl->scene);




}

RoutineEditPanel::~RoutineEditPanel()
{
    // Detach from the viewer first - Panel::~Panel() destroys m_impl->viewer
    // (a QWidget child) after this body returns, and ~GraphWidget() dereferences
    // whatever scene it still thinks it has.
    m_impl->viewer->setScene(nullptr);
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

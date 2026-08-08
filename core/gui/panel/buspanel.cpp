#include <QVBoxLayout>
#include "buspanel.h"
#include "view/graphwidget.h"
#include "view/scene.h"
#include "project/project.h"
#include "graph/bus/busgraph.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

class BusPanel::Impl
{
public:
    keira::GraphWidget *viewer;
    keira::Scene *scene = nullptr;
};

BusPanel::BusPanel() : Panel("photon.bus"),m_impl(new Impl)
{

    m_impl->viewer = new keira::GraphWidget(photonApp->plugins()->nodeLibrary());
    setPanelWidget(m_impl->viewer);
    setName("Bus");


}

BusPanel::~BusPanel()
{
    // projectWillClose() only fires on an explicit project switch - app shutdown
    // deletes the project directly without it, so the scene (and its live
    // GraphEvaluator eval thread) must also be cleaned up here, or a shutdown
    // with a project still open leaks a thread that ticks a freed BusGraph.
    // Detach from the viewer first - Panel::~Panel() destroys m_impl->viewer
    // (a QWidget child) after this body returns, and ~GraphWidget() dereferences
    // whatever scene it still thinks it has.
    m_impl->viewer->setScene(nullptr);
    delete m_impl->scene;
    delete m_impl;
}

void BusPanel::projectDidOpen(photon::Project* project)
{
    m_impl->scene = new keira::Scene;
    m_impl->scene->setIsAutoEvaluate(false);
    m_impl->scene->setGraph(project->bus());
    m_impl->scene->setNodeLibrary(photonApp->plugins()->nodeLibrary());


    m_impl->viewer->setScene(m_impl->scene);
}

void BusPanel::projectWillClose(photon::Project* project)
{
    m_impl->viewer->setScene(nullptr);
    delete m_impl->scene;
    m_impl->scene = nullptr;
}


} // namespace photon

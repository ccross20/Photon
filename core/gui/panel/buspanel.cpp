#include <QVBoxLayout>
#include "buspanel.h"
#include "view/viewer.h"
#include "view/scene.h"
#include "project/project.h"
#include "graph/bus/busgraph.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

class BusPanel::Impl
{
public:
    keira::Viewer *viewer;
    keira::Scene *scene = nullptr;
};

BusPanel::BusPanel() : Panel("photon.bus"),m_impl(new Impl)
{

    m_impl->viewer = new keira::Viewer;
    setPanelWidget(m_impl->viewer);
    setName("Bus");


}

BusPanel::~BusPanel()
{
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
    if(m_impl->scene)
        delete m_impl->scene;
}


} // namespace photon

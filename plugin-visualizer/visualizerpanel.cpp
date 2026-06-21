#include "visualizerpanel.h"
#include "photoncore.h"
#include "project/project.h"
#include "scene/sceneobject.h"
#include "graph/bus/busevaluator.h"
#include "data/dmxmatrix.h"
#include "rhi/rhiviewport.h"

namespace photon {

VisualizerPanel::VisualizerPanel() : Panel("Visualizer")
{
    m_viewport = new RhiViewport;

    if (photonApp->project())
        m_viewport->setSceneRoot(photonApp->project()->sceneRoot());

    setPanelWidget(m_viewport);

    connect(photonApp->busEvaluator(), &BusEvaluator::evaluationCompleted,
            this, &VisualizerPanel::tick);
}

VisualizerPanel::~VisualizerPanel()
{
}

void VisualizerPanel::projectDidOpen(Project *project)
{
    m_viewport->setSceneRoot(project->sceneRoot());
    connect(m_viewport, &RhiViewport::selectionChanged,
            project, &Project::setSelectedSceneObject);
    connect(project, &Project::selectedSceneObjectChanged,
            m_viewport, &RhiViewport::setSelectedSceneObject);
}

void VisualizerPanel::projectWillClose(Project *project)
{
    disconnect(m_viewport, &RhiViewport::selectionChanged,
               project, &Project::setSelectedSceneObject);
    disconnect(project, &Project::selectedSceneObjectChanged,
               m_viewport, &RhiViewport::setSelectedSceneObject);
    m_viewport->setSceneRoot(nullptr);
}

void VisualizerPanel::tick()
{
    m_viewport->setDmxState(photonApp->busEvaluator()->dmxMatrix());
}

} // namespace photon

#include "visualizerpanel.h"
#include "photoncore.h"
#include "project/project.h"
#include "scene/sceneobject.h"
#include "graph/bus/busevaluator.h"
#include "data/dmxmatrix.h"
#include "rhi/rhiviewport.h"

namespace photon {

// The id MUST match the one this panel is registered under in
// PluginVisualizer::initialize (and the one GuiManager's "Open Visualizer"
// action asks for). saveLayout() persists panel->id() and restoreLayout()
// feeds it straight back to createPanel() as the lookup key, so a mismatch
// saves fine and then restores as a "panel not found" placeholder.
VisualizerPanel::VisualizerPanel() : Panel("visualizer")
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
            project, &Project::setSelectedSceneObjects);
    connect(project, &Project::selectedSceneObjectsChanged,
            m_viewport, &RhiViewport::setSelectedSceneObjects);
}

void VisualizerPanel::projectWillClose(Project *project)
{
    disconnect(m_viewport, &RhiViewport::selectionChanged,
               project, &Project::setSelectedSceneObjects);
    disconnect(project, &Project::selectedSceneObjectsChanged,
               m_viewport, &RhiViewport::setSelectedSceneObjects);
    m_viewport->setSceneRoot(nullptr);
}

void VisualizerPanel::tick()
{
    m_viewport->setDmxState(photonApp->busEvaluator()->dmxMatrix());
}

} // namespace photon

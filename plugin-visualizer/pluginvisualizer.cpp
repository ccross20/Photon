#include "pluginvisualizer.h"
#include "plugin/pluginfactory.h"
#include "visualizerpanel.h"

inline void initPluginResource() { Q_INIT_RESOURCE(resources); }

namespace photon {

bool PluginVisualizer::initialize(const PluginContext &context)
{
    initPluginResource();
    Q_UNUSED(context)

    //initPluginResource();

    // The default QSurfaceFormat used to be set here too. It belongs in main()
    // (photon-desktop/desktop.cpp), which runs before the QApplication exists -
    // setting it here, at plugin-load time, was already too late to affect the
    // platform integration, and the viewport now renders through QRhi anyway.

    photonApp->plugins()->registerPluginPanel("visualizer",[](){return new VisualizerPanel;});


    //exoApp->plugins()->registerPluginPanel(ViewportPanel::IdType,[](){return new ViewportPanel();});



    return true;
}

QVersionNumber PluginVisualizer::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginVisualizer::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginVisualizer::name()
{
    return "Visualization";
}

QString PluginVisualizer::description()
{
    return "Adds OpenGL Visualizatio";
}

QString PluginVisualizer::id()
{
    return "photon.opengl-visualizer";
}


} // photon

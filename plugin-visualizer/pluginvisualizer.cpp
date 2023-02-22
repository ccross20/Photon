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


    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);


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
    return "OpenGL Visualization";
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

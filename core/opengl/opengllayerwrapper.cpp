#include "opengllayerwrapper.h"

namespace exo {


OpenGLLayerWrapper::OpenGLLayerWrapper(OpenGLViewportLayer *layer, OpenGLViewport *viewport) : OpenGLLayer(layer, viewport),m_layer(layer)
{
    layer->setOpenGLViewport(viewport);
}

void OpenGLLayerWrapper::initGL(QOpenGLContext *context)
{
    m_layer->initGL(context);
}

void OpenGLLayerWrapper::draw(QOpenGLContext *context)
{
    markClean();
    m_layer->markClean();
    m_layer->draw(context);
}

} // namespace exo

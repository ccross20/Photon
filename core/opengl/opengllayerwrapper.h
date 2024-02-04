#ifndef DECO_OPENGLLAYERWRAPPER_H
#define DECO_OPENGLLAYERWRAPPER_H

#include "opengllayer.h"
#include "openglviewportlayer.h"

namespace exo {

class OpenGLLayerWrapper : public OpenGLLayer
{
public:
    OpenGLLayerWrapper(OpenGLViewportLayer *layer, OpenGLViewport *viewport);

    void initGL(QOpenGLContext *) override;
    void draw(QOpenGLContext *) override;

private:
    OpenGLViewportLayer *m_layer;
};

} // namespace exo

#endif // DECO_OPENGLLAYERWRAPPER_H

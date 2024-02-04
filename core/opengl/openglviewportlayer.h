#ifndef DECO_OPENGLVIEWPORTLAYER_H
#define DECO_OPENGLVIEWPORTLAYER_H
#include <QOpenGLContext>
#include "viewport/layer/viewportlayer.h"

namespace exo {

class OpenGLViewport;

class OpenGLViewportLayer : public ViewportLayer
{
public:
    OpenGLViewportLayer(const LayerTypeId &layerType, QObject *parent = nullptr);
    virtual ~OpenGLViewportLayer();

    virtual void initGL(QOpenGLContext *){}
    virtual void draw(QOpenGLContext *context) = 0;

    OpenGLViewport *openGLViewport() const{return m_openGLViewport;}
    void setOpenGLViewport(OpenGLViewport *vp){m_openGLViewport = vp;}

private:
    OpenGLViewport *m_openGLViewport;
};

} // namespace exo

#endif // DECO_OPENGLVIEWPORTLAYER_H

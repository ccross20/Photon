#ifndef OPENGLLAYERPAINTER_H
#define OPENGLLAYERPAINTER_H

#include <QPainter>
#include "openglshader.h"
#include "opengltexture.h"
#include "openglplane.h"
#include "event/ieventlistener.h"
#include "opengllayer.h"
#include "raster/raster.h"

namespace exo {

class OpenGLLayerPainter : public OpenGLLayer
{
public:
    OpenGLLayerPainter(ViewportLayerPtr layer, OpenGLViewport *viewport);
    ~OpenGLLayerPainter() override;


    void initGL(QOpenGLContext *) override;
    void draw(QOpenGLContext *) override;
    void receiveEvent(int eventType, void *source, void *data1, void *data2) override;
    void markDirty() override;

private:
    void paintLayer();

    int m_camMatrixLoc;
    int m_viewMatrixLoc;
    OpenGLShader *m_shader;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
    //QImage m_pixelData;
    Raster* m_raster;
};

} // namespace exo

#endif // OPENGLLAYERPAINTER_H

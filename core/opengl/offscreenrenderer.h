#ifndef DECO_OFFSCREENRENDERER_H
#define DECO_OFFSCREENRENDERER_H
#include <QOpenGLExtraFunctions>
#include <QImage>
#include "photon-global.h"
#include "openglshader.h"

namespace photon {

class OffscreenRenderer : public QOpenGLExtraFunctions
{
public:
    OffscreenRenderer(QOpenGLContext *context);
    ~OffscreenRenderer();

    void setShader(const OpenGLShader &shader);
    void init(uint width, uint height, QImage::Format format);
    uint newFrameBufferTexture();

    void setTexture(uint tex);
    void copyToTexture(uint dst);
    void loadTexture(QImage *raster);
    void writeToRaster(QImage *raster, const bounds_i &bounds = bounds_i{});

    void popBuffer();
    void popFrontBuffer();

    uint frameBufferId() const;
    uint frameBufferTexture() const;
    void draw();

private:
    OpenGLShader m_shader;
    uint m_frameBufferId;
    uint m_frameBufferTexture;
    QVector<uint> m_textures;
    QVector<uint> m_buffers;
    uint m_vbo;
    uint m_ebo;
    uint m_vao;
    uint m_tex;
    uint m_width;
    uint m_height;
};

} // namespace exo

#endif // DECO_OFFSCREENRENDERER_H

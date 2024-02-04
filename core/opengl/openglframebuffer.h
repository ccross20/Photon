#ifndef DECO_OPENGLFRAMEBUFFER_H
#define DECO_OPENGLFRAMEBUFFER_H

#include <QOpenGLExtraFunctions>
#include "opengltexture.h"
#include "data/types.h"

namespace photon {

class PHOTONCORE_EXPORT OpenGLFrameBuffer : public QOpenGLExtraFunctions
{
public:
    OpenGLFrameBuffer(OpenGLTexture *texture, QOpenGLContext *context, const point_d offset = point_d{});
    ~OpenGLFrameBuffer();
    void destroy();
    void resize(int width, int height);

    void setOffset(const point_d t_offset);
    point_d offset() const;
    void setTexture(OpenGLTexture *texture);
    void setDepthTexture(OpenGLTexture *texture);
    void bind();
    void unbind();
    int handle() const{return m_frameBufferHandle;}
    OpenGLTexture *texture();
    int width() const{return m_width;}
    int height() const{return m_height;}
    void writeToRaster(QImage *raster, const bounds_i &bounds = bounds_i{}, bool swapRGB = false);

private:
    void updateViewport();
    point_d m_offset;
    int m_width = 0;
    int m_height = 0;
    OpenGLTexture *m_texture;
    uint m_frameBufferHandle = 0;
};

} // namespace exo

#endif // DECO_OPENGLFRAMEBUFFER_H

#ifndef PHOTON_TEXTURE_H
#define PHOTON_TEXTURE_H

#include "plugin-visualizer-global.h"
#include <QImage>

namespace photon {

class Texture
{
public:
    Texture();
    ~Texture();
    void destroy();
    bool isValid() const;
    void bind(QOpenGLContext *context = nullptr);
    void unbind();
    void generateMip();
    int width() const{return m_width;}
    int height() const{return m_height;}
    int channelCount() const{return m_num_channel;}
    uint handle() const{return m_handle;}
    void setTarget(int target){m_target = target;}
    void setFilter(int filter){m_minFilter = filter;m_magFilter = filter;}
    void setMinFilter(int filter){m_minFilter = filter;}
    void setMagFilter(int filter){m_magFilter = filter;}
    int minFilter() const{return m_minFilter;}
    int magFilter() const{return m_magFilter;}
    void setWrap(int wrap){m_wrap = wrap;}
    void resize(QOpenGLContext *context, int internalFormat,
                int w, int h, int format, int type = GL_UNSIGNED_BYTE, int num_channel = 4, int byte_per_channel = 1, const void *data = nullptr);
    void resize(QOpenGLContext *context, QImage::Format format, int w, int h, const void *data = nullptr);
    void resize(QOpenGLContext *context, const QImage &image, const void *data = nullptr);
    void resize(int w, int h);
    void loadImage(QOpenGLContext *context, const QImage &image);

private:
    QOpenGLContext *m_context = nullptr;
    int m_handle = -1;
    int m_width = 0;
    int m_height = 0;
    int m_target = GL_TEXTURE_2D;
    int m_internalFormat = GL_RGBA8;
    int m_format = GL_RGBA;
    int m_type = GL_UNSIGNED_BYTE;
    int m_minFilter = GL_LINEAR;
    int m_magFilter = GL_LINEAR;
    int m_wrap = GL_CLAMP_TO_EDGE;
    int m_num_channel = 4;
    int m_byte_per_channel = 1;
};

} // namespace photon

#endif // PHOTON_TEXTURE_H

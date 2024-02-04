#include "openglframebuffer.h"
#include <QImage>

namespace photon {

OpenGLFrameBuffer::OpenGLFrameBuffer(OpenGLTexture *texture, QOpenGLContext *context, const point_d offset) : QOpenGLExtraFunctions(context),m_offset(offset),m_texture(texture)
{
    m_width = m_texture->width();
    m_height = m_texture->height();
    glGenFramebuffers(1, &m_frameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
    updateViewport();
    m_texture->bind(context);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture->handle(), 0);

}

OpenGLFrameBuffer::~OpenGLFrameBuffer()
{
    destroy();
}

void OpenGLFrameBuffer::destroy()
{
    if(m_frameBufferHandle > 0)
    {
        glDeleteFramebuffers(1, &m_frameBufferHandle);
        m_frameBufferHandle = 0;
    }
}

void OpenGLFrameBuffer::updateViewport()
{
   //glViewport(-m_offset.x, m_offset.y, m_width, m_height);
   glViewport(0,0, m_width, m_height);
}

void OpenGLFrameBuffer::setOffset(const point_d t_offset)
{
    m_offset = t_offset;

    updateViewport();
}

point_d OpenGLFrameBuffer::offset() const
{
    return m_offset;
}

void OpenGLFrameBuffer::resize(int width, int height)
{
    if(m_height == height && m_width == width)
        return;

    m_width = width;
    m_height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
    updateViewport();
    m_texture->resize(m_width, m_height);
    setTexture(m_texture);

}

void OpenGLFrameBuffer::setTexture(OpenGLTexture *texture)
{
    m_texture = texture;
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
    m_texture->bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture->handle(), 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLFrameBuffer::setDepthTexture(OpenGLTexture *texture)
{
    m_texture = texture;
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
    m_texture->bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texture->handle(), 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLFrameBuffer::writeToRaster(QImage *raster, const bounds_i &bounds, bool swapRGB)
{
    int glFormat = -1;
    int glType = -1;
    switch (raster->format()) {
        default:
    case QImage::Format_ARGB32_Premultiplied:
            glPixelStorei(GL_PACK_ALIGNMENT,4);
            glType = GL_UNSIGNED_BYTE;
            glFormat = swapRGB ? GL_BGRA : GL_RGBA;
            break;

    }

    //if(glFormat < 0)
        //return;

    if(bounds.is_valid())
    {
        Q_ASSERT(bounds.width() == static_cast<int>(raster->width()));
        Q_ASSERT(bounds.height() == static_cast<int>(raster->height()));

        glReadPixels(bounds.left(), bounds.top(), bounds.width(), bounds.height(),  glFormat, glType, raster->bits());
    }
    else
        glReadPixels(0, 0, raster->width(), raster->height(),  glFormat, glType, raster->bits());

}

void OpenGLFrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferHandle);
    updateViewport();
}

void OpenGLFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLTexture *OpenGLFrameBuffer::texture()
{
    return m_texture;
}

} // namespace exo

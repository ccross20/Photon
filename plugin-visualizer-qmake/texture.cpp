#include <QOpenGLFunctions>
#include "texture.h"

namespace photon {

Texture::Texture()
{

}

Texture::~Texture()
{
    destroy();
}

void Texture::destroy()
{
    if(isValid() && m_context)
    {
        uint tex[1] = {static_cast<uint>(m_handle)};
        m_context->functions()->glDeleteTextures(1, tex);

        m_handle = 0;
        m_width = 0;
        m_height = 0;
    }
}

void Texture::bind(QOpenGLContext *context)
{
    if(context)
        m_context = context;
    if(isValid() && m_context)
        m_context->functions()->glBindTexture(m_target, m_handle);
}

void Texture::unbind()
{
    if(isValid() && m_context)
        m_context->functions()->glBindTexture(m_target, 0);
}

bool Texture::isValid() const
{
    return m_handle != 0;
}

void Texture::resize(QOpenGLContext *context, QImage::Format format, int w, int h, const void *data)
{
    if(m_width == w && m_height == h && !data)
        return;

    switch (format) {
    default:
    case QImage::Format_ARGB32:
            resize(context, GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_BYTE,4,1, data);
        break;

    }
}

void Texture::resize(QOpenGLContext *context, const QImage &image, const void *data)
{
    resize(context, image.format(), image.width(), image.height(), data);
}

void Texture::resize(QOpenGLContext *context, int internalFormat,
            int w, int h, int format, int type, int num_channel, int byte_per_channel, const void *data)
{

    m_context = context;
    m_width = w;
    m_height = h;
    m_format = format;
    m_internalFormat = internalFormat;
    m_type = type;
    m_num_channel = num_channel;
    m_byte_per_channel = byte_per_channel;
    auto f = m_context->functions();

    if(m_handle < 0)
    {
        uint handle;
        f->glGenTextures(1, &handle);
        m_handle = handle;
    }

    f->glBindTexture(m_target, m_handle);
    f->glTexImage2D(m_target, 0, m_internalFormat, m_width, m_height, 0, m_format, m_type, data);


/*
    GLenum error = f->glGetError();
    if(error)
    {
        switch (error) {
            case GL_INVALID_VALUE:
                qDebug() << "Invalid Value";
            break;
            case GL_INVALID_ENUM :
                qDebug() << "Invalid Enum";
            break;
            case GL_INVALID_OPERATION :
                qDebug() << "Invalid Operation";
            break;
        }

        if(m_target != GL_TEXTURE_2D)
            qDebug() << "Not texture 2D";

        qDebug() << m_width << "   " << m_height;
        qDebug() << "Opengl Error: " << error;
    }
*/

    //if(format == GL_BGRA || format == GL_RGBA)
    {
        f->glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrap);
        f->glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_wrap);

        f->glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter);
        f->glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter);
        //f->glTexParameterfv(GL_TEXTURE_BORDER_COLOR, GL_TEXTURE_MAG_FILTER, new float[4]{0,0,0,0});
    }

    f->glBindTexture(m_target, 0);
}

void Texture::resize(int w, int h)
{
    if(m_width == w && m_height == h)
        return;
    resize(m_context, m_internalFormat, w, h, m_format, m_type, m_num_channel, m_byte_per_channel);
}

void Texture::generateMip()
{
    if(!isValid())
        return;
    auto f = m_context->functions();
    f->glBindTexture(m_target, m_handle);
    m_minFilter = GL_LINEAR_MIPMAP_LINEAR;
    m_magFilter = GL_LINEAR;
    f->glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter);
    f->glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter);
    f->glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::loadImage(QOpenGLContext *context, const QImage &image)
{
    resize(context, image.format(), image.width(), image.height(), image.constBits());
}

} // namespace photon

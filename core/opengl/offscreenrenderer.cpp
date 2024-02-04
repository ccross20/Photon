#include "offscreenrenderer.h"

namespace photon {

OffscreenRenderer::OffscreenRenderer(QOpenGLContext *context) : QOpenGLExtraFunctions(context)
{

}

OffscreenRenderer::~OffscreenRenderer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);

    glDeleteTextures(m_textures.length(), m_textures.constData());
    glDeleteFramebuffers(m_buffers.length(), m_buffers.constData());

}

void OffscreenRenderer::setShader(const OpenGLShader &shader)
{
    m_shader = shader;
}

void OffscreenRenderer::popFrontBuffer()
{
    if(m_buffers.isEmpty() || m_textures.isEmpty())
        return;

    uint frontBuffer = m_buffers.front();
    glDeleteFramebuffers(1, &frontBuffer);
    m_buffers.pop_front();

    uint frontTexture = m_textures.front();
    uint tex[1] = {frontTexture};
    glDeleteTextures(2, tex);
    m_textures.pop_front();
}

void OffscreenRenderer::popBuffer()
{
    if(m_buffers.isEmpty() || m_textures.isEmpty())
        return;

    uint frontBuffer = m_buffers.back();
    glDeleteFramebuffers(1, &frontBuffer);
    m_buffers.pop_back();

    uint frontTexture = m_textures.back();
    uint tex[1] = {frontTexture};
    glDeleteTextures(2, tex);
    m_textures.pop_back();
}

uint OffscreenRenderer::newFrameBufferTexture()
{
    glGenFramebuffers(1, &m_frameBufferId);
    glGenTextures(1, &m_frameBufferTexture);


    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

    glBindTexture(GL_TEXTURE_2D, m_frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_MIRRORED_REPEAT);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_frameBufferTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_buffers.append(m_frameBufferId);
    m_textures.append(m_frameBufferTexture);

    return m_frameBufferTexture;

}

void OffscreenRenderer::setTexture(uint tex)
{
    m_frameBufferTexture = tex;

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
    glBindTexture(GL_TEXTURE_2D, tex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_frameBufferTexture, 0);
    glDrawBuffers(1,&m_frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OffscreenRenderer::copyToTexture(uint dst)
{
    glBindTexture(GL_TEXTURE_2D, dst);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, m_width, m_height, 0);

}

void OffscreenRenderer::init(uint width, uint height, QImage::Format format)
{
    initializeOpenGLFunctions();


    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
    //glBlendEquation (GL_FUNC_ADD);
    //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    m_width = width;
    m_height = height;

    newFrameBufferTexture();


    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER) ) {
            case GL_FRAMEBUFFER_UNDEFINED:
                qDebug() << "FB undefined";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                qDebug() << "FB attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                qDebug() << "FB missing attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                qDebug() << "FB draw buffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                qDebug() << "FB read buffer";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                qDebug() << "FB unsupported";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                qDebug() << "FB multisample";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                qDebug() << "FB layer targets";
                break;
        }
        qDebug() << "Frame buffer incomplete " << glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
    }


    glBindTexture(GL_TEXTURE_2D, m_frameBufferTexture);
    glEnable(GL_TEXTURE_2D);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
    glViewport(0, 0, width, height);

    float vertices[] = {-1.f, 1.f,0.f,   -0.f, 1.f,
                       1.f, 1.f,0.f,     1.f, 1.f,
                       -1.f, -1.f,0.f,   -0.f, -0.f,
                       1.f, -1.f, 0.f,   1.f, -0.f};



    uint indices[] = {0,1,2,3};

    glGenBuffers(1, &m_vbo);

    glGenBuffers(1, &m_ebo);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);


    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

#if defined (Q_OS_WIN)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#elif defined (Q_OS_MAC)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#endif

}


void OffscreenRenderer::loadTexture(QImage *raster)
{
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    switch (raster->format()) {
    case QImage::Format_ARGB32_Premultiplied:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raster->width(), raster->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, raster->constBits());
            break;
        default:
            break;

    }


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    m_textures.append(m_tex);


}

void OffscreenRenderer::writeToRaster(QImage *raster, const bounds_i &bounds)
{
    int glFormat = -1;
    switch (raster->format()) {
        case QImage::Format_ARGB32_Premultiplied:
            glFormat = GL_RGBA;
            break;
        default:
            break;

    }

    if(glFormat < 0)
        return;

    if(bounds.is_valid())
    {
        glReadPixels(raster->rect().left(), raster->rect().top(), raster->width(), raster->height(),  glFormat, GL_UNSIGNED_BYTE, raster->bits());
    }
    else
        glReadPixels(0, 0, raster->width(), raster->height(),  glFormat, GL_UNSIGNED_BYTE, raster->bits());



}

void OffscreenRenderer::draw()
{
   // m_shader.bind();

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
}

uint OffscreenRenderer::frameBufferId() const
{
    return m_frameBufferId;
}

uint OffscreenRenderer::frameBufferTexture() const
{
    return m_frameBufferTexture;
}

} // namespace exo

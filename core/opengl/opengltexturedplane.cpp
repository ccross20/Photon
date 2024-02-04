#include <QOpenGLFunctions>
#include "opengltexturedplane.h"

namespace photon {

OpenGLTexturedPlane::OpenGLTexturedPlane(QOpenGLContext *context, const bounds_d &bounds, bool flipY):m_context(context),
    m_texture(-1),
    m_bounds(bounds),
    m_flipY(flipY)
{

}

OpenGLTexturedPlane::~OpenGLTexturedPlane()
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    if(!context)
        return;
    auto f = context->extraFunctions();

    if(m_texture >= 0)
    {
        uint texs[1] = {static_cast<uint>(m_texture)};
        f->glDeleteTextures(1, texs);

        m_texture = -1;
    }


    f->glDeleteVertexArrays(1, &m_vao);
    f->glDeleteBuffers(1, &m_vbo);
    f->glDeleteBuffers(1, &m_ebo);
}

void OpenGLTexturedPlane::create(GLfloat depth)
{

    bounds_f bnds = m_bounds;

    float y1 = bnds.y1;
    float y2 = bnds.y2;

    if(m_flipY)
    {
        y1 = bnds.y2;
        y2 = bnds.y1;
    }

    float vertices[] = {bnds.x1, y1,m_depth,   -0.f, 1.f,
                       bnds.x2, y1,m_depth,     1.f, 1.f,
                       bnds.x1, y2,m_depth,   -0.f, -0.f,
                       bnds.x2, y2, m_depth,   1.f, -0.f};

    m_depth = depth;
    uint indices[] = {0,1,2,3};

    QOpenGLContext *context = QOpenGLContext::currentContext();
    auto f = context->extraFunctions();

    f->glGenBuffers(1, &m_vbo);
    f->glGenBuffers(1, &m_ebo);
    f->glGenVertexArrays(1, &m_vao);

    f->glBindVertexArray(m_vao);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

#if defined (Q_OS_WIN)
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#elif defined (Q_OS_MAC)
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#endif


}

void OpenGLTexturedPlane::setBounds(const bounds_d &bounds)
{

    m_bounds = bounds;
    bounds_f bnds = m_bounds;

    float y1 = bnds.y1;
    float y2 = bnds.y2;

    if(m_flipY)
    {
        y1 = bnds.y2;
        y2 = bnds.y1;
    }

    float vertices[] = {bnds.x1, y1,m_depth,   -0.f, 1.f,
                       bnds.x2, y1,m_depth,     1.f, 1.f,
                       bnds.x1, y2,m_depth,   -0.f, -0.f,
                       bnds.x2, y2, m_depth,   1.f, -0.f};


    uint indices[] = {0,1,2,3};

    auto f = m_context->extraFunctions();


    f->glBindVertexArray(m_vao);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

#if defined (Q_OS_WIN)
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#elif defined (Q_OS_MAC)
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#endif

}

void OpenGLTexturedPlane::setImage(const QImage &img)
{

    auto f = m_context->extraFunctions();
    bool remakeTexture = true;

    if(m_texture >= 0)
    {
        if(QSize(m_textureWidth, m_textureHeight) != img.size())
        {
            uint texs[1] = {static_cast<uint>(m_texture)};
            f->glDeleteTextures(1, texs);

            m_texture = -1;
        } else {
            remakeTexture = false;
        }
    }


    if(remakeTexture)
    {
        uint t;
        f->glGenTextures(1, &t);
        m_texture = t;
    }

    m_textureWidth = img.width();
    m_textureHeight = img.height();
    f->glBindTexture(GL_TEXTURE_2D, m_texture);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());

    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

void OpenGLTexturedPlane::updateImage(const QImage &raster, const bounds_i &bounds)
{
    if(m_texture < 0 || bounds == m_bounds)
    {
        qDebug() << "full update";
        setImage(raster);
        return;
    }

    auto f = m_context->extraFunctions();

    f->glBindTexture(GL_TEXTURE_2D, m_texture);
    f->glTexSubImage2D(GL_TEXTURE_2D, 0, bounds.left(), bounds.top(), bounds.width(), bounds.height(), GL_BGRA, GL_UNSIGNED_BYTE, raster.bits());
}

void OpenGLTexturedPlane::setTexture(int textureId)
{
    m_texture = textureId;
}

void OpenGLTexturedPlane::destroy()
{

    //m_texture->destroy();
    /*
    if(m_texture >= 0)
    {
        uint texs[1] = {static_cast<uint>(m_texture)};
        glDeleteTextures(1, texs);

        m_texture = -1;
    }
*/
}

void OpenGLTexturedPlane::preDraw(QOpenGLContext *)
{



}

void OpenGLTexturedPlane::setMinFormat(int format)
{
    m_minFormat = format;
}

void OpenGLTexturedPlane::setMagFormat(int format)
{
    m_magFormat = format;
}


void OpenGLTexturedPlane::draw(QOpenGLContext *context)
{
    auto f = context->extraFunctions();
    f->glBindVertexArray(m_vao);

    f->glBindTexture(GL_TEXTURE_2D, m_texture);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFormat);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFormat);

    f->glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
    f->glBindTexture(GL_TEXTURE_2D, 0);

}

void OpenGLTexturedPlane::postDraw(QOpenGLContext *)
{

}

} // namespace exo

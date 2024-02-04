#include "openglplane.h"

namespace photon {

OpenGLPlane::OpenGLPlane(QOpenGLContext *context, const bounds_d &bounds, bool flipY, bool flipX): m_context(context),m_bounds(bounds),m_flipY(flipY),m_flipX(flipX)
{
    auto f = context->extraFunctions();

    f->glGenBuffers(1, &m_vbo);
    f->glGenBuffers(1, &m_ebo);
    f->glGenVertexArrays(1, &m_vao);

    setBounds(bounds);
    m_isValid = true;
}

OpenGLPlane::~OpenGLPlane()
{
    destroy();
}

void OpenGLPlane::destroy()
{
    if(!m_isValid)
        return;

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if(!context)
        return;
    auto f = context->extraFunctions();

    f->glDeleteVertexArrays(1, &m_vao);
    f->glDeleteBuffers(1, &m_vbo);
    f->glDeleteBuffers(1, &m_ebo);
}

void OpenGLPlane::setBounds(const bounds_d &bounds)
{
    m_bounds = bounds;

    float y1 = m_bounds.y1;
    float y2 = m_bounds.y2;


    float v1 = 1.0f;
    float v2 = 0.0f;
    float w1 = 0.0f;
    float w2 = 1.0f;

    if(m_flipY)
    {
        v1 = 0.0f;
        v2 = 1.0f;
        //std::swap(y1,y2);
    }
    if(m_flipX)
    {
        w1 = 1.0f;
        w2 = 0.0f;
    }


    bounds_f boundsF = m_bounds;

    float vertices[] = {boundsF.x1, y1,m_depth,   w1, v1,
                       boundsF.x2, y1,m_depth,     w2, v1,
                       boundsF.x1, y2,m_depth,   w1, v2,
                       boundsF.x2, y2, m_depth,   w2, v2};

    m_depth = 0.0;
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

void OpenGLPlane::setPoints(point_f ptA, point_f ptB, point_f ptC, point_f ptD)
{

    float v1 = 1.0f;
    float v2 = 0.0f;
    float w1 = 0.0f;
    float w2 = 1.0f;

    if(m_flipY)
    {
        v1 = 0.0f;
        v2 = 1.0f;
        std::swap(ptA, ptD);
        std::swap(ptB, ptC);
    }
    if(m_flipX)
    {
        w1 = 1.0f;
        w2 = 0.0f;
    }
    m_bounds = bounds_d{ptA, ptB};
    m_bounds.unite(ptC);
    m_bounds.unite(ptD);

    float vertices[] = {ptA.x, ptA.y,m_depth,   w1, v1,
                       ptB.x, ptB.y,m_depth,     w2, v1,
                       ptC.x, ptC.y,m_depth,   w1, v2,
                       ptD.x, ptD.y, m_depth,   w2, v2};

    m_depth = 0.0;
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

void OpenGLPlane::bind(QOpenGLContext *context)
{
    m_context = context;
}

void OpenGLPlane::draw(QOpenGLContext *context)
{
    auto f = context->extraFunctions();
    f->glBindVertexArray(m_vao);
    f->glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
}

void OpenGLPlane::draw()
{
    auto f = m_context->extraFunctions();
    f->glBindVertexArray(m_vao);
    f->glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
}


} // namespace exo

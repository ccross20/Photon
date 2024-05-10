#include "openglresources.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"

namespace photon {

class OpenGLResources::Impl
{
public:
    QOpenGLContext *context;
    OpenGLShader *basicShader;
    OpenGLShader *projShader;
    OpenGLPlane *plane;
    int viewportLoc;
    int cameraLoc;
};

OpenGLResources::OpenGLResources():m_impl(new Impl)
{

}

OpenGLResources::~OpenGLResources()
{
    delete m_impl;
}

void OpenGLResources::init(QOpenGLContext *t_context)
{
    m_impl->basicShader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                     ":/resources/shader/texture.frag");
    m_impl->projShader = new OpenGLShader(t_context, ":/resources/shader/projectedvertex.vert",
                                           ":/resources/shader/texture.frag");
    m_impl->projShader->bind(t_context);
    m_impl->viewportLoc = m_impl->projShader->uniformLocation("projMatrix");
    m_impl->cameraLoc = m_impl->projShader->uniformLocation("mvMatrix");

    m_impl->plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
}

void OpenGLResources::destroy(QOpenGLContext *t_context)
{
    m_impl->basicShader->bind(t_context);
    delete m_impl->basicShader;
    m_impl->projShader->bind(t_context);
    delete m_impl->projShader;

    m_impl->plane->bind(t_context);
    m_impl->plane->destroy();
}

void OpenGLResources::bind(QOpenGLContext *t_context)
{
    m_impl->context = t_context;
}

void OpenGLResources::drawPlane() const
{
    m_impl->plane->draw();
}

void OpenGLResources::bindBasicShader(int textureHandle)
{
    m_impl->basicShader->bind(m_impl->context);
    m_impl->basicShader->setTexture("tex",textureHandle);
}

void OpenGLResources::bindProjectedShader(int textureHandle, const QMatrix4x4 &viewportMatrix, const QMatrix4x4 &cameraMatrix)
{
    m_impl->projShader->bind(m_impl->context);
    m_impl->projShader->setTexture("tex",textureHandle);
    m_impl->projShader->setMatrix(m_impl->viewportLoc, viewportMatrix);
    m_impl->projShader->setMatrix(m_impl->cameraLoc, cameraMatrix);
}

OpenGLShader *OpenGLResources::basicShader() const
{
    return m_impl->basicShader;
}

OpenGLShader *OpenGLResources::projectedShader() const
{
    return m_impl->projShader;
}

void OpenGLResources::clear(float r, float g, float b, float a) const
{
    m_impl->context->functions()->glClearColor(.0f,.0f,.0f,.0f);
    m_impl->context->functions()->glClear(GL_COLOR_BUFFER_BIT);
}


} // namespace photon

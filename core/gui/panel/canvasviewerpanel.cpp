#include <QLabel>
#include "canvasviewerpanel.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/canvascollection.h"
#include "pixel/canvas.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "opengl/openglframebuffer.h"

namespace photon {


OpenGLPreviewWidget::OpenGLPreviewWidget(Canvas *t_canvas, QWidget *t_parent) : QOpenGLWidget(t_parent),m_canvas(t_canvas)
{
    m_bgColor = Qt::red;
}

OpenGLPreviewWidget::~OpenGLPreviewWidget()
{
    makeCurrent();
    m_shader->bind(context());
    m_plane->bind(context());
    delete m_shader;
    delete m_plane;
    doneCurrent();
}

void OpenGLPreviewWidget::initializeGL()
{

    initializeOpenGLFunctions();
    m_plane = new OpenGLPlane(context(), bounds_d{-1,-1,1,1}, false);
    m_shader = new OpenGLShader(context(), ":/resources/shader/BasicTextureVertex.vert",
                                ":/resources/shader/texture.frag");
}

void OpenGLPreviewWidget::paintGL()
{

    glViewport(0,0,width(), height());
    glClearColor(m_bgColor.red(), m_bgColor.green(), m_bgColor.blue(), m_bgColor.alpha());
    glClear(GL_COLOR_BUFFER_BIT);


    glEnable(GL_BLEND);
    if(m_canvas->texture())
    {

#if defined(Q_OS_WIN)
        //glViewport(0,0,width(), height());
#elif defined(Q_OS_MAC)
        //glViewport(0,0,width()*2.0, height()*2.0);
#endif
        glViewport(0,0,width(), height());
        m_shader->bind(context());
        m_canvas->texture()->bind(context());
        m_shader->setTexture("tex",m_canvas->texture()->handle());
        m_plane->draw(context());
    }

}

void OpenGLPreviewWidget::resizeGL(int w, int h)
{

}



class CanvasViewerPanel::Impl
{
public:
    OpenGLPreviewWidget *previewWidget;
};

CanvasViewerPanel::CanvasViewerPanel() : Panel("photon.canvas-viewer"),m_impl(new Impl)
{
    Canvas *canvas = nullptr;
    if(photonApp->project() && photonApp->project()->canvases()->canvasCount() > 0)
        canvas = photonApp->project()->canvases()->canvases()[0];
    if(canvas)
    {
        connect(canvas, &Canvas::textureDidUpdate, this, &CanvasViewerPanel::canvasUpdated);

    }
    m_impl->previewWidget = new OpenGLPreviewWidget(canvas, nullptr);
    setPanelWidget(m_impl->previewWidget);


}

CanvasViewerPanel::~CanvasViewerPanel()
{
    delete m_impl;
}

void CanvasViewerPanel::canvasUpdated()
{
    m_impl->previewWidget->update();
}

} // namespace photon

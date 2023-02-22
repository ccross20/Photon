#include <QMouseEvent>
#include <QPaintEvent>
#include "openglviewport.h"
#include "scene.h"
#include "camera.h"

namespace photon {

OpenGLViewport::OpenGLViewport(QWidget *parent) : QOpenGLWidget (parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    //m_camera.setPosition(QVector3D(0,0,150));
    //m_camera.setDistance(150);
}

OpenGLViewport::~OpenGLViewport()
{
    disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLViewport::cleanup);
}

void OpenGLViewport::setScene(Scene *t_scene)
{
    m_scene = t_scene;
}

void OpenGLViewport::cleanup()
{


    makeCurrent();

    //do stuff

    doneCurrent();

    m_openGLInitialized = false;
}
void OpenGLViewport::timerEvent(QTimerEvent *)
{
    if(!m_scene)
        return;
    makeCurrent();

    m_scene->destroy(context());
    m_scene->create(context());
    m_scene->rebuild(context());

    doneCurrent();
    update();

}

void OpenGLViewport::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLViewport::cleanup);

    initializeOpenGLFunctions();
    //glClearColor(.2f, .2f, .2f, 1);
    glClearColor(1.f, 1.f, 1.f, 1);
    glBlendEquation (GL_FUNC_ADD);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_DEPTH_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);


    //QString renderString = QString(gl_version);

    const std::string vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const std::string renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const std::string version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    m_version = QString::fromStdString(version);
    m_vendor = QString::fromStdString(vendor);
    m_model = QString::fromStdString(renderer);
    //program->release();

    m_openGLInitialized = true;

    m_timer.start(16, this);



}

void OpenGLViewport::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    if(!m_scene)
        return;
    m_scene->camera()->setAspect(aspect);

}

void OpenGLViewport::paintEvent(QPaintEvent *e)
{
    QOpenGLWidget::paintEvent(e);

}

void OpenGLViewport::paintGL()
{

    const QColor &bgColor = Qt::gray;
    glClearColor(static_cast<GLclampf>(bgColor.redF()), static_cast<GLclampf>(bgColor.greenF()), static_cast<GLclampf>(bgColor.blueF()), 1);
    //glClearColor(1, 0,0, 1);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendEquation (GL_FUNC_ADD);
    glEnable(GL_CULL_FACE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);


    if(!m_scene)
        return;

    DrawContext *drawContext = new DrawContext(context(), m_scene->camera());

    m_scene->draw(drawContext);

}

void OpenGLViewport::resizeEvent(QResizeEvent *event)
{

    QOpenGLWidget::resizeEvent(event);
}

void OpenGLViewport::enterEvent(QEnterEvent *event)
{

}

void OpenGLViewport::leaveEvent(QEvent *event)
{

}

void OpenGLViewport::keyPressEvent(QKeyEvent *event)
{
    if(!m_scene)
        return;
    m_scene->camera()->keyPressEvent(event);
}

void OpenGLViewport::keyReleaseEvent(QKeyEvent *event)
{
    if(!m_scene)
        return;
    m_scene->camera()->keyReleaseEvent(event);
}

void OpenGLViewport::mousePressEvent(QMouseEvent *event)
{
    if(!m_scene)
        return;
    QOpenGLWidget::mousePressEvent(event);

    m_scene->camera()->mousePressEvent(event);
}

void OpenGLViewport::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_scene)
        return;
    QOpenGLWidget::mouseMoveEvent(event);

    m_scene->camera()->mouseMoveEvent(event);



    //m_lastPosition = event->pos();

}

void OpenGLViewport::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_scene)
        return;
    QOpenGLWidget::mouseReleaseEvent(event);
    m_scene->camera()->mouseReleaseEvent(event);
}

void OpenGLViewport::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(!m_scene)
        return;
    QOpenGLWidget::mouseDoubleClickEvent(event);
    m_scene->camera()->mouseDoubleClickEvent(event);
}

void OpenGLViewport::wheelEvent(QWheelEvent *event)
{
    if(!m_scene)
        return;
    QOpenGLWidget::wheelEvent(event);
    m_scene->camera()->wheelEvent(event);


    update();
}

void OpenGLViewport::dragEnterEvent(QDragEnterEvent *event)
{

}

void OpenGLViewport::dragMoveEvent(QDragMoveEvent *event)
{

}

void OpenGLViewport::dropEvent(QDropEvent *event)
{

}

void OpenGLViewport::dragLeaveEvent(QDragLeaveEvent *event)
{

}


} // namespace photon

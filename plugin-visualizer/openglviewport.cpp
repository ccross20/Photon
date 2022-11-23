#include <QMouseEvent>
#include <QPaintEvent>
#include "openglviewport.h"

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

void OpenGLViewport::setRootEntity(Entity *t_entity)
{
    m_rootEntity = t_entity;
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
    if(!m_rootEntity)
        return;
    makeCurrent();

    m_rootEntity->destroy(context());
    m_rootEntity->create(context());
    m_rootEntity->rebuild(context());

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

    m_timer.start(20, this);



}

void OpenGLViewport::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    m_camera.setAspect(aspect);

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

    if(!m_rootEntity)
        return;

    DrawContext *drawContext = new DrawContext(context(), &m_camera);

    m_rootEntity->draw(drawContext);

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
    switch (event->key()) {
    case Qt::Key_Left:
        m_camera.ProcessKeyboard(LEFT, 1);
        update();
        break;
    case Qt::Key_Right:
        m_camera.ProcessKeyboard(RIGHT, 1);
        update();
        break;
    case Qt::Key_Up:
        m_camera.ProcessKeyboard(FORWARD, 1);
        update();
        break;
    case Qt::Key_Down:
        m_camera.ProcessKeyboard(BACKWARD, 1);
        update();
        break;
    default:
        break;
    }
}

void OpenGLViewport::keyReleaseEvent(QKeyEvent *event)
{

}

void OpenGLViewport::mousePressEvent(QMouseEvent *event)
{
    QOpenGLWidget::mousePressEvent(event);
    m_lastPosition = event->pos();

    QVector3D pt(m_lastPosition.x(), m_lastPosition.y(), -1);
    auto mat = m_camera.matrix();
    m_startWorldPoint =  mat.map(pt);

    if(event->buttons() & Qt::LeftButton)
        m_camera.StartOrbit();
    if(event->buttons() & Qt::RightButton)
        m_camera.StartOrbit(m_camera.position());
}

void OpenGLViewport::mouseMoveEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseMoveEvent(event);

    if(event->buttons() & Qt::RightButton)
    {
        QPointF delta = event->pos() - m_lastPosition;

        m_camera.FreeRotate(delta.x(), delta.y());
        //m_camera.ProcessMouseMovement(delta.x(), delta.y());

        update();
    }

    if(event->buttons() & Qt::LeftButton)
    {
        QPointF delta = event->pos() - m_lastPosition;

        m_camera.Orbit(delta.x(), delta.y());

        update();
    }


    if(event->buttons() & Qt::MiddleButton)
    {
        QPointF delta = event->pos() - m_lastPosition;

        m_camera.ProcessMousePan(delta.x(), delta.y());


        m_lastPosition = event->pos();
        update();
    }

    //m_lastPosition = event->pos();

}

void OpenGLViewport::mouseReleaseEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseReleaseEvent(event);
}

void OpenGLViewport::mouseDoubleClickEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void OpenGLViewport::wheelEvent(QWheelEvent *event)
{
    m_camera.ProcessMouseScroll(event->angleDelta().y()/5.0);


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

#include <QMouseEvent>
#include <QPaintEvent>
#include "openglviewport.h"
#include "viewport/layer/viewportlayer.h"
#include "exocore.h"
#include "opengllayerpainter.h"
#include "openglsubdividedlayerpainter.h"
#include "opengllayerwrapper.h"
#include "viewport/gpupixelselectionlayer.h"

namespace exo {


OpenGLViewport::OpenGLViewport(QWidget *parent) : QOpenGLWidget (parent),m_controller(new ViewportController(this))
{
    m_controller->init();

    /*
    GPUPixelSelectionLayer *pixelLayer = new GPUPixelSelectionLayer;
    pixelLayer->init();
    m_controller->addLayer(pixelLayer);
    */

    setMouseTracking(true);
    setTabletTracking(true);
    connect(m_controller, SIGNAL(layerAdded(ViewportLayerPtr,ViewportLayerPtr)),this,SLOT(layerAdded(ViewportLayerPtr,ViewportLayerPtr)));
    connect(m_controller, SIGNAL(layerRemoved(ViewportLayerPtr)),this,SLOT(layerRemoved(ViewportLayerPtr)));
    connect(m_controller,SIGNAL(redraw(bounds_i)),this,SLOT(redrawViewport(bounds_i)));
    connect(m_controller, SIGNAL(offsetWasSet(point_d)),this,SLOT(offsetWasSet(point_d)));
    connect(m_controller, SIGNAL(scaleWasSet(double)),this,SLOT(scaleWasSet(double)));
    connect(m_controller, SIGNAL(rotationWasSet(double)),this,SLOT(rotationWasSet(double)));
    connect(m_controller, SIGNAL(cursorSet(QCursor)),this,SLOT(cursorSet(QCursor)));
    connect(m_controller, SIGNAL(sceneBoundsChanged(bounds_d)),this,SLOT(sceneBoundsChanged(bounds_d)));
    connect(&m_resizeTimer,SIGNAL(timeout()),this,SLOT(resizeTimerFire()));
    m_resizeTimer.setInterval(200);
    m_resizeTimer.setSingleShot(true);



    setAcceptDrops(true);
    animatedScale = m_controller->scale();
    scaleAnimation = new QPropertyAnimation(this, "animateScale");
    scaleAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    scaleAnimation->setDuration(200);
    connect(scaleAnimation,SIGNAL(finished()),this,SLOT(animationComplete()));

    //setUpdateBehavior(QOpenGLWidget::PartialUpdate);
}

OpenGLViewport::~OpenGLViewport()
{
    delete m_controller;
    disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLViewport::cleanup);
}

void OpenGLViewport::redrawViewport(const bounds_i &rect)
{
    update(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
}

point_i OpenGLViewport::sceneOffset() const
{
    return m_controller->sceneOffset();
}

void OpenGLViewport::buildTransform()
{
    point_i sceneCenter = m_controller->sceneCenter();
    point_d offset = m_controller->offset();
    double rotation = m_controller->rotation();
    double scale = animatedScale;

    m_transform = trans_affine();
    m_transform.translate(-sceneCenter.x, -sceneCenter.y);
    m_transform.rotate(deg2rad(rotation));
    m_transform.translate(m_controller->offset());
    //m_transform.translate(sceneOff);
    m_transform.scale(scale);
    m_transform.translate(m_controller->center());

    m_cameraMatrix.setToIdentity();
    //m_cameraMatrix.translate(-sceneOff.x,sceneOff.y);
    m_cameraMatrix.rotate(rotation,0,0,1);
    m_cameraMatrix.scale(static_cast<float>(scale), static_cast<float>(scale));
    m_cameraMatrix.translate(-sceneCenter.x, -sceneCenter.y);

   // m_cameraMatrix.translate(sceneOff.x, sceneOff.y);

    m_cameraMatrix.translate(static_cast<float>(offset.x), static_cast<float>(offset.y));


    //m_cameraMatrix.translate(static_cast<float>(sceneOff.x), static_cast<float>(sceneOff.y));

    //qDebug() << sceneCenter << "    " << offset;


    m_cursorCameraMatrix.setToIdentity();
    m_cursorCameraMatrix.scale(static_cast<float>(scale), static_cast<float>(scale));


    m_viewportScaleMatrix.setToIdentity();
    if(isAnimating)
        m_viewportScaleMatrix.scale(static_cast<float>(scale), static_cast<float>(scale));

    m_controller->setTransform(m_transform);

}

trans_affine OpenGLViewport::transform() const
{
    return m_transform;
}

void OpenGLViewport::sceneBoundsChanged(const bounds_d &bounds)
{
    Q_UNUSED(bounds)

    buildTransform();
}

void OpenGLViewport::offsetWasSet(const point_d &offset)
{
    Q_UNUSED(offset)
    buildTransform();

    for(auto layer : m_layers)
    {
        //if(layer->layer()->coordinateSpace() != ViewportLayer::SceneSpace)
            layer->offset(offset);
    }

    update();
}

void OpenGLViewport::scaleWasSet(double scale)
{
    Q_UNUSED(scale)

    animatedScale = scale;
    buildTransform();

    for(auto layer : m_layers)
        layer->scale(scale);

    update();

    /*
    scaleAnimation->stop();
    scaleAnimation->setKeyValueAt(0.0, animatedScale);
    scaleAnimation->setKeyValueAt(1.0, scale);
    scaleAnimation->start();
    isAnimating = true;

    */
}

void OpenGLViewport::rotationWasSet(double t_rotation)
{
    buildTransform();
    for(auto layer : m_layers)
        layer->rotate(t_rotation);

    update();
}

double OpenGLViewport::animateScale() const
{
    return animatedScale;
}

void OpenGLViewport::setAnimateScale(double value)
{
    animatedScale = value;
    buildTransform();

    /*
    for(auto layer : m_layers)
        layer->markDirty();
        */

    update();
}

void OpenGLViewport::animationComplete()
{
    isAnimating = false;
    buildTransform();

    for(auto layer : m_layers)
        layer->markDirty();

    update();
}

void OpenGLViewport::cursorSet(const QCursor &cursor)
{
    setCursor(cursor);
}

size_i OpenGLViewport::contentSize() const
{
    return m_controller->sceneBounds().size();
}

QWidget *OpenGLViewport::widget()
{
    return this;
}

OpenGLLayer *OpenGLViewport::createLayer(ViewportLayerPtr layer)
{
    OpenGLLayer *resultLayer = nullptr;
    OpenGLViewportLayer *oglLayer = dynamic_cast<OpenGLViewportLayer*>(layer);
    if(oglLayer)
    {
        resultLayer = new OpenGLLayerWrapper(oglLayer, this);

    }
    else
    {
        if(layer->isSubdividable())
            resultLayer = new OpenGLSubdividedLayerPainter(layer, this);
        else
            resultLayer = new OpenGLLayerPainter(layer, this);
    }

    resultLayer->initGL(context());
    return resultLayer;
}

void OpenGLViewport::layerAdded(ViewportLayerPtr layer, ViewportLayerPtr before)
{
    if(m_openGLInitialized)
    {


        makeCurrent();

        if(before)
        {
            for(auto it = m_layers.begin(); it != m_layers.end(); ++it)
            {
                if((*it)->layer() == before)
                {
                    m_layers.insert(it, createLayer(layer));
                    break;
                }
            }
        }
        else
            m_layers.append(createLayer(layer));

        doneCurrent();
        update();
    }
}

void OpenGLViewport::layerRemoved(ViewportLayerPtr layer)
{
    for(auto checkLayer : m_layers)
    {
        if(checkLayer->layer() == layer)
        {

            m_layers.removeOne(checkLayer);
            makeCurrent();
            delete checkLayer;

            doneCurrent();
            update();
            return;
        }
    }
}

void OpenGLViewport::cleanup()
{


    makeCurrent();

    for(OpenGLLayer *layer : m_layers)
        delete layer;
    m_layers.clear();

    doneCurrent();

    m_openGLInitialized = false;
}

QString OpenGLViewport::info(ViewportInformation info) const
{
    switch (info) {
        case IViewport::InfoVendor:
            return m_vendor;
        case IViewport::InfoVersion:
            return m_version;
        case IViewport::InfoModel:
            return m_model;
    }
    return "";
}

void OpenGLViewport::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLViewport::cleanup);

    initializeOpenGLFunctions();
    //glClearColor(.2f, .2f, .2f, 1);
    glClearColor(1.f, 1.f, 1.f, 1);
    glBlendEquation (GL_FUNC_ADD);
    glEnable(GL_BLEND);
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


    for(ViewportLayerPtr layer : m_controller->layers())
    {
        //if(layer->coordinateSpace() == ViewportLayer::SceneSpace)
            m_layers.append(createLayer(layer));
    }



}

void OpenGLViewport::paintEvent(QPaintEvent *e)
{
    QOpenGLWidget::paintEvent(e);

    m_paintRect = e->rect();
}

void OpenGLViewport::paintGL()
{

    const Color &bgColor = m_controller->backgroundColor();
    glClearColor(static_cast<GLclampf>(bgColor.red()), static_cast<GLclampf>(bgColor.green()), static_cast<GLclampf>(bgColor.blue()), 1);
    //glClearColor(1, 0,0, 1);

    glClear(GL_COLOR_BUFFER_BIT);

    glBlendEquation (GL_FUNC_ADD);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);



    for(OpenGLLayer *layer : m_layers)
    {
        if(!layer->layer()->isEnabled())
            continue;

        //glViewport(0,0,width(), height());
        //glViewport(0,0,width()*2.0, height()*2.0);
        /*
        if(layer->layer()->coordinateSpace() == ViewportLayer::CursorSpace)
        {

            //if(mousePosition.x() < 0 || mousePosition.x() > 2 || mousePosition.y() > 0 || mousePosition.y() < -2)
                //continue;
            QMatrix4x4 mat = m_cursorCameraMatrix;
            //mat.translate(.5,.5);
            mat *= m_cursorMatrix;
            //mat.translate(-(static_cast<double>(layer->layer()->bounds().width())/m_controller->bounds().width()), static_cast<double>(layer->layer()->bounds().height())/m_controller->bounds().height());
            m_sceneShader->setMatrix(m_camMatrixLoc, mat);


            QMatrix4x4 viewMat;
            viewMat.ortho(QRect(0,0,m_controller->bounds().width(),m_controller->bounds().height()));

            m_sceneShader->setMatrix(m_viewMatrixLoc, viewMat);
        }
        */
        layer->draw(context());
    }

}

void OpenGLViewport::resizeEvent(QResizeEvent *event)
{
    m_newSize = event->size();
    //m_resizeTimer.start();
    resizeTimerFire();

    //setUpdatesEnabled(false);
}

void OpenGLViewport::resizeTimerFire()
{
    QResizeEvent event(m_newSize,m_oldSize);
    m_oldSize = m_newSize;

    QOpenGLWidget::resizeEvent(&event);
    m_controller->resizeEvent(&event);
    point_i center = m_controller->center();

    m_viewportMatrix.setToIdentity();
    m_viewportMatrix.ortho(QRect(-center.x,-center.y,m_controller->bounds().width(),m_controller->bounds().height()));
    //m_viewportMatrix.ortho(QRect(-m_controller->bounds().width(),-m_controller->bounds().height(),m_controller->bounds().width(),m_controller->bounds().height()));

    buildTransform();


    //setUpdatesEnabled(true);

}

void OpenGLViewport::updateViewport(const QRect &bounds)
{
    Q_UNUSED(bounds)
    update();
    /*
    if(!bounds.isValid())
        update();
    else
        update(bounds.adjusted(-10,-10,10,10));
        */
    //qDebug() << "updateViewport " << bounds;
}


QSize OpenGLViewport::sizeHint() const
{
    return m_controller->sceneBounds().size().toQSize();
}

void OpenGLViewport::enterEvent(QEnterEvent *event)
{
    m_controller->enterEvent(event);
}

void OpenGLViewport::leaveEvent(QEvent *event)
{
    m_controller->leaveEvent(event);
}

void OpenGLViewport::keyPressEvent(QKeyEvent *event)
{
    m_controller->keyPressEvent(event);
}

void OpenGLViewport::keyReleaseEvent(QKeyEvent *event)
{
    m_controller->keyReleaseEvent(event);
}

void OpenGLViewport::tabletEvent(QTabletEvent *event)
{
    if(event->buttons() & Qt::MiddleButton || exoApp->keyIsDown(Qt::Key_Space))
        return;


    m_cursorMatrix.setToIdentity();
    m_cursorMatrix.translate(event->pos().x(), event->pos().y());

    m_cursorLocation = event->pos();
    switch (event->type()) {
        case QEvent::TabletPress:
        {
            event->accept();
            setFocus();
            ViewportMouseEvent e(event, m_controller->transform().inverted().mapPoint(point_d(event->pos())));
            m_controller->mousePressEvent(&e);
            m_tabletInUse = true;
        }
            break;
        case QEvent::TabletMove:
        {
        //m_tabletInUse = true;
            if(!m_tabletInUse)
                m_tabletHover = true;
            else
                event->accept();

            auto t = QDateTime::currentMSecsSinceEpoch();

            if(!m_tabletInUse)
            {
                m_tabletHover = true;
                if(t - m_lastTabletTime < 30)
                    return;
            } else if(t - m_lastTabletTime < 10)
                return;


            m_lastTabletTime = t;

            //qDebug() << rad2deg(atan2(event->yTilt(), event->xTilt())) << event->xTilt() << event->yTilt();


            //if(m_lastTabletPos.x() == event->pos().x() && m_lastTabletPos.y() == event->pos().y())
                //return;

            ViewportMouseEvent e(event, m_controller->transform().inverted().mapPoint(point_d(event->pos())));
            m_lastTabletPos = event->pos();
            if(!m_tabletInUse)
                e.clearButtons();
            m_controller->mouseMoveEvent(&e);
        }
        break;
        case QEvent::TabletRelease:
        {
            event->accept();
            ViewportMouseEvent e(event, m_controller->transform().inverted().mapPoint(point_d(event->pos())));
            m_controller->mouseReleaseEvent(&e);
            m_tabletInUse = false;
            m_delayedTabletRelease = true;
        }
        break;
        default:
            return;
    }
}

void OpenGLViewport::mousePressEvent(QMouseEvent *event)
{
    if(m_tabletInUse || m_delayedTabletRelease)
        return;
    setFocus();
    m_cursorMatrix.setToIdentity();
    m_cursorMatrix.translate(event->pos().x(), event->pos().y());
    QOpenGLWidget::mousePressEvent(event);

    ViewportMouseEvent e(event, m_controller, event->pos());
    m_controller->mousePressEvent(&e);

}

void OpenGLViewport::mouseMoveEvent(QMouseEvent *event)
{
    if(m_tabletHover)
    {
        m_tabletHover = false;
        return;
    }
    if(m_tabletInUse)
        return;

    if(m_delayedTabletRelease)
    {
        m_delayedTabletRelease = false;
        return;
    }


    QOpenGLWidget::mouseMoveEvent(event);

    m_cursorMatrix.setToIdentity();

    //qDebug() << event->pos();
    m_cursorMatrix.translate(event->pos().x(), event->pos().y());
    ViewportMouseEvent e(event, m_controller, event->pos());
    m_controller->mouseMoveEvent(&e);

    //update();
}

void OpenGLViewport::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_delayedTabletRelease)
    {
        m_delayedTabletRelease = false;
        return;
    }
    if(m_tabletInUse)
        return;
    QOpenGLWidget::mouseReleaseEvent(event);
    ViewportMouseEvent e(event, m_controller, event->pos());
    m_controller->mouseReleaseEvent(&e);
}

void OpenGLViewport::mouseDoubleClickEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseDoubleClickEvent(event);
    ViewportMouseEvent e(event, m_controller, event->pos());
    m_controller->mouseDoubleClickEvent(&e);
}

void OpenGLViewport::wheelEvent(QWheelEvent *event)
{
    m_controller->wheelEvent(event);
}

void OpenGLViewport::dragEnterEvent(QDragEnterEvent *event)
{
    m_controller->dragEnterEvent(event);
}

void OpenGLViewport::dragMoveEvent(QDragMoveEvent *event)
{
    m_controller->dragMoveEvent(event);
}

void OpenGLViewport::dropEvent(QDropEvent *event)
{
    m_controller->dropEvent(event);
}

void OpenGLViewport::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_controller->dragLeaveEvent(event);
}



} // namespace exo

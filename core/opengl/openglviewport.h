#ifndef OPENGLVIEWPORT_H
#define OPENGLVIEWPORT_H

#include <QPropertyAnimation>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QEvent>
#include <QTimer>
#include <QResizeEvent>
#include "exo-core_global.h"
#include "event/ieventlistener.h"
#include "viewport/viewportcontroller.h"
#include "viewport/iviewport.h"
#include "openglshader.h"

namespace exo {

class OpenGLLayerPainter;
class OpenGLLayer;
class OpenGLViewportLayer;

class OpenGLViewport : public QOpenGLWidget, public IViewport, protected QOpenGLFunctions
{
    Q_OBJECT
    Q_PROPERTY(double animateScale READ animateScale WRITE setAnimateScale)
public:

    OpenGLViewport(QWidget *parent = nullptr);
    virtual ~OpenGLViewport() override;

    trans_affine transform() const override;
    size_i contentSize() const override;
    QWidget *widget() override;

    QString info(ViewportInformation info) const override;

    OpenGLLayer *createLayer(ViewportLayerPtr layer);
    ViewportController *controller() const override{ return m_controller; }

    void updateViewport(const QRect &bounds = QRect());
    QSize sizeHint() const override;

    double animateScale() const;
    void setAnimateScale(double value);

    point_i sceneOffset() const;

    QMatrix4x4 cameraMatrix() const{return m_cameraMatrix;}
    QMatrix4x4 viewportMatrix() const{return m_viewportMatrix;}

public slots:
    void cleanup();

private slots:
    void layerAdded(ViewportLayerPtr layer, ViewportLayerPtr before);
    void layerRemoved(ViewportLayerPtr layer);
    void redrawViewport(const bounds_i &rect);
    void offsetWasSet(const point_d &offset);
    void scaleWasSet(double scale);
    void rotationWasSet(double t_degrees);
    void cursorSet(const QCursor &cursor);
    void sceneBoundsChanged(const bounds_d &bounds);
    void animationComplete();
    void resizeTimerFire();

protected:
    void initializeGL() override;
    void paintGL() override;

    void paintEvent(QPaintEvent *e) override;

    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;    
    void tabletEvent(QTabletEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;


private:
    friend class ViewportPanel;

    void buildTransform();

    int m_camMatrixLoc;
    int m_viewMatrixLoc;
    QMatrix4x4 m_cameraMatrix;
    QMatrix4x4 m_cursorCameraMatrix;
    QMatrix4x4 m_tempCameraMatrix;
    QMatrix4x4 m_viewportMatrix;
    QMatrix4x4 m_cursorMatrix;
    QMatrix4x4 m_viewportScaleMatrix;
    trans_affine m_transform;
    QVector<OpenGLLayer*> m_layers;
    double animatedScale = 1.0;
    QPropertyAnimation *scaleAnimation;
    bool isAnimating = false;

    QString m_vendor;
    QString m_version;
    QString m_model;
    QRect m_paintRect;
    QSize  m_oldSize;
    QSize  m_newSize;
    QTimer m_resizeTimer;
    QColor m_viewportBGColor;
    QColor m_sceneBGColor;
    QPoint m_cursorLocation;
    QPoint m_lastTabletPos;
    long long m_lastTabletTime;
    bool m_openGLInitialized = false;
    bool m_tabletInUse = false;
    bool m_tabletHover = false;
    bool m_delayedTabletRelease = false;

    ViewportController *m_controller;
};

} // namespace exo

#endif // OPENGLVIEWPORT_H

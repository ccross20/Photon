#ifndef PHOTON_OPENGLVIEWPORT_H
#define PHOTON_OPENGLVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QBasicTimer>
#include <QMatrix4x4>
#include "freecamera.h"
#include "entity.h"

namespace photon {

class OpenGLViewport : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    OpenGLViewport(QWidget *parent = nullptr);
    virtual ~OpenGLViewport() override;

public slots:
    void cleanup();
    void setRootEntity(photon::Entity *entity);

protected:
    void timerEvent(QTimerEvent *e) override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void paintEvent(QPaintEvent *e) override;

    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
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
    QBasicTimer m_timer;
    FreeCamera m_camera;
    QString m_vendor;
    QString m_version;
    QString m_model;
    QPointF m_lastPosition;
    QVector3D m_startWorldPoint;
    Entity *m_rootEntity = nullptr;
    bool m_openGLInitialized = false;
};

} // namespace photon

#endif // PHOTON_OPENGLVIEWPORT_H

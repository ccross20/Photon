#ifndef PHOTON_OPENGLVIEWPORT_H
#define PHOTON_OPENGLVIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QBasicTimer>
#include <QMatrix4x4>

namespace photon {

class Scene;

class OpenGLViewport : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    OpenGLViewport(QWidget *parent = nullptr);
    virtual ~OpenGLViewport() override;

public slots:
    void cleanup();
    void setScene(photon::Scene *scene);

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
    QString m_vendor;
    QString m_version;
    QString m_model;
    QPointF m_lastPosition;
    QVector3D m_startWorldPoint;
    Scene *m_scene = nullptr;
    bool m_openGLInitialized = false;
};

} // namespace photon

#endif // PHOTON_OPENGLVIEWPORT_H

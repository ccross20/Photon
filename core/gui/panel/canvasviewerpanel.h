#ifndef PHOTON_CANVASVIEWERPANEL_H
#define PHOTON_CANVASVIEWERPANEL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "photon-global.h"
#include "gui/panel.h"


namespace photon {

class PHOTONCORE_EXPORT OpenGLPreviewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    OpenGLPreviewWidget(Canvas *, QWidget *);
    ~OpenGLPreviewWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    QColor m_bgColor = Qt::white;
    OpenGLShader *m_shader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    Canvas *m_canvas = nullptr;
};





class PHOTONCORE_EXPORT CanvasViewerPanel : public Panel
{
    Q_OBJECT
public:
    CanvasViewerPanel();
    ~CanvasViewerPanel();

private slots:
    void canvasUpdated();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASVIEWERPANEL_H

#ifndef CANVASACTION_H
#define CANVASACTION_H
#include <QWidget>
#include "surfaceaction.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasAction : public SurfaceAction
{
    Q_OBJECT
public:
    CanvasAction(QObject *parent = nullptr);
    virtual ~CanvasAction();

    void setCanvas(Canvas *);
    Canvas *canvas() const;

    void addPixelLayout(PixelLayout *);
    void removePixelLayout(PixelLayout *);
    PixelLayout *pixelLayoutAtIndex(int index) const;
    int pixelLayoutCount() const;
    const QVector<PixelLayout*> &pixelLayouts() const;
    QOpenGLContext *openGLContext() const;

    virtual void processChannels(ProcessContext &) override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    QWidget *settingsWidget() const override;

signals:

    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);

private slots:
    void canvasSizeUpdated(QSize);

private:

    Canvas *m_canvas = nullptr;
    QVector<PixelLayout*> m_pixelLayouts;
    QOpenGLContext *m_context = nullptr;
    int m_canvasIndex = -1;

    OpenGLShader *m_shader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
    int m_viewportLoc;
    int m_cameraLoc;
};

} // namespace photon

#endif // CANVASACTION_H

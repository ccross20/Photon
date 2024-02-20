#ifndef PHOTON_CANVASCLIP_H
#define PHOTON_CANVASCLIP_H
#include <QOpenGLContext>
#include "clip.h"


namespace photon {


class PHOTONCORE_EXPORT CanvasClip : public Clip
{
    Q_OBJECT
public:
    CanvasClip();

    CanvasClip(double start, double duration, QObject *parent = nullptr);
    ~CanvasClip();

    void processChannels(ProcessContext &) override;

    virtual void initializeContext(QOpenGLContext *, Canvas *);
    virtual void canvasResized(QOpenGLContext *, Canvas *);

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    static ClipInformation info();

protected:
    void layerDidChange(Layer*) override;

private:
    class Impl;
    Impl *m_impl;
};


} // namespace photon

#endif // PHOTON_CANVASCLIP_H

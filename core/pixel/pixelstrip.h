#ifndef PHOTON_PIXELSTRIP_H
#define PHOTON_PIXELSTRIP_H

#include <QWidget>
#include "photon-global.h"
#include "pixelsource.h"
#include "scene/sceneobject.h"

namespace photon {

class PixelStrip;

class PixelStripEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PixelStripEditorWidget(PixelStrip *,QWidget *parent = nullptr);
    ~PixelStripEditorWidget();

signals:

private slots:
    void setName(const QString &name);
    void setDMXOffset(int);
    void setUniverse(int);
    void setPixelCount(int);
    void setLength(float);
    void setCenter(float);
    void setAngle(float);
    void setBend(float);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);


private:
    class Impl;
    Impl *m_impl;
};





class PHOTONCORE_EXPORT PixelStrip : public SceneObject, public PixelSource
{
public:
    PixelStrip(SceneObject *parent = nullptr);
    ~PixelStrip();

    // Kept for the visualizer's future use (physical strip geometry) - no
    // longer drives pixel-mapping-canvas positions, which live on
    // PixelSourceLayout instead. See pixelCount() for the DMX/canvas concern.
    double center() const;
    double angle() const;
    double bend() const;
    double length() const;

    void setLength(double);
    void setCenter(double);
    void setAngle(double);
    void setBend(double);
    void setPixelCount(int);
    void setDMXOffset(int offset);
    void setUniverse(int universe);

    QWidget *createEditor() override;
    int pixelCount() const override;

    QByteArray sourceUniqueId() const override;
    int dmxOffset() const override;
    int dmxSize() const override;
    int universe() const override;

    void process(ProcessContext &, const QVector<QPointF> &positions, double blend=1.0) const override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELSTRIP_H

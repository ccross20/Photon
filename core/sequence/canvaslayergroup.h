#ifndef PHOTON_CANVASLAYERGROUP_H
#define PHOTON_CANVASLAYERGROUP_H
#include <QWidget>
#include "layergroup.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasLayerGroupEditor : public QWidget
{
    Q_OBJECT
public:
    CanvasLayerGroupEditor(CanvasLayerGroup *);


public slots:
    void canvasChanged(int);
    void openAddPixelLayout();
    void removeSelectedLayout();
    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT CanvasLayerGroup : public LayerGroup
{
    Q_OBJECT
public:
    CanvasLayerGroup(QObject *parent = nullptr);
    CanvasLayerGroup(Canvas *, const QString &name);
    ~CanvasLayerGroup();

    void setCanvas(Canvas *);
    Canvas *canvas() const;

    void addPixelLayout(PixelLayout *);
    void removePixelLayout(PixelLayout *);
    PixelLayout *pixelLayoutAtIndex(int index) const;
    int pixelLayoutCount() const;
    const QVector<PixelLayout*> &pixelLayouts() const;
    QOpenGLContext *openGLContext() const;

    QVector<PixelSource*> sources() const;

    QWidget *createEditor() override;
    void processChannels(ProcessContext &) override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

signals:

    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);

private slots:
    void canvasSizeUpdated(QSize);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASLAYERGROUP_H

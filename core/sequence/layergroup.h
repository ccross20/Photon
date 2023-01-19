#ifndef PHOTON_LAYERGROUP_H
#define PHOTON_LAYERGROUP_H
#include "layer.h"

namespace photon {

class PHOTONCORE_EXPORT LayerGroup : public Layer
{
    Q_OBJECT
public:
    LayerGroup(const QByteArray &layerType, QObject *parent = nullptr);
    LayerGroup(const QString &name, const QByteArray &layerType, QObject *parent = nullptr);
    virtual ~LayerGroup();

    void addLayer(Layer *);
    void removeLayer(Layer *);
    Layer *findLayerByGuid(const QUuid &guid) override;
    const QVector<Layer*> &layers() const;

    virtual bool isGroup() const override{return true;}
    virtual int height() const override;
    virtual void processChannels(ProcessContext &) override;
    virtual void restore(Project &) override;
    virtual void readFromJson(const QJsonObject &, const LoadContext &) override;
    virtual void writeToJson(QJsonObject &) const override;

signals:
    void layerUpdated(photon::Layer *);
    void layerAdded(photon::Layer *);
    void layerRemoved(photon::Layer *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LAYERGROUP_H

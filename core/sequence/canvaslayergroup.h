#ifndef PHOTON_CANVASLAYERGROUP_H
#define PHOTON_CANVASLAYERGROUP_H
#include "layergroup.h"

namespace photon {

class CanvasLayerGroup : public LayerGroup
{
public:
    CanvasLayerGroup(QObject *parent = nullptr);
    CanvasLayerGroup(Canvas *, const QString &name);
    ~CanvasLayerGroup();

    void setCanvas(Canvas *);
    Canvas *canvas() const;

    void processChannels(ProcessContext &) override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASLAYERGROUP_H

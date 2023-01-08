#ifndef PHOTON_MASTERLAYER_H
#define PHOTON_MASTERLAYER_H

#include <QObject>
#include "layer.h"

namespace photon {

class PHOTONCORE_EXPORT MasterLayer : public Layer
{
public:
    MasterLayer(QObject *parent = nullptr);
    MasterLayer(const QString &name, QObject *parent = nullptr);
    virtual ~MasterLayer();

    Channel *channel() const;

    double process(double time) const;
    virtual void processChannels(ProcessContext &) override;
    virtual void readFromJson(const QJsonObject &, const LoadContext &) override;
    virtual void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MASTERLAYER_H

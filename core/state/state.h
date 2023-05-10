#ifndef PHOTON_STATE_H
#define PHOTON_STATE_H
#include "photon-global.h"
#include "fixture/capability/fixturecapability.h"
#include "stateevaluationcontext.h"

namespace photon {

class PHOTONCORE_EXPORT State : public QObject
{
    Q_OBJECT
public:
    State();
    ~State();

    void addDefaultCapabilities();
    QByteArray uniqueId() const;
    virtual void initializeValues(StateEvaluationContext &) const;
    virtual void evaluate(const StateEvaluationContext &) const;
    StateCapability *addCapability(CapabilityType);
    void removeCapability(StateCapability *);
    const QVector<StateCapability*> &capabilities() const;

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    void channelAdded(int index);
    void channelRemoved(int index);
    void channelUpdated(int index);
    void capabilityAdded(photon::StateCapability *);
    void capabilityRemoved(photon::StateCapability *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_STATE_H

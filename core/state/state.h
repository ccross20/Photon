#ifndef PHOTON_STATE_H
#define PHOTON_STATE_H
#include "photon-global.h"
#include "fixture/capability/fixturecapability.h"
#include "stateevaluationcontext.h"

namespace photon {

// Not a ProjectResource: a State is always privately owned by whatever uses it
// (a FixtureStateNode in the graph, a FixtureClip) rather than living in a
// project-wide, browsable collection.
class PHOTONCORE_EXPORT State : public QObject
{
    Q_OBJECT
public:
    State();
    ~State();

    QString name() const;
    void setName(const QString &);
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
    void metadataUpdated();
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

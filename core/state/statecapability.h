#ifndef PHOTON_STATECAPABILITY_H
#define PHOTON_STATECAPABILITY_H

#include "fixture/capability/fixturecapability.h"
#include "stateevaluationcontext.h"

namespace photon {

class PHOTONCORE_EXPORT StateCapability
{
public:
    StateCapability(CapabilityType);
    virtual ~StateCapability();

    CapabilityType capabilityType() const;
    virtual CapabilityType fixtureCapabilityType() const;

    QByteArray uniqueId() const;
    QVector<ChannelInfo> availableChannels() const;
    virtual void initializeValues(StateEvaluationContext &) const;
    virtual void evaluate(const StateEvaluationContext &) const;

    bool isEnabled() const;
    void setIsEnabled(bool);

    QString name() const;
    void setName(const QString &);
    float getChannelFloat(const StateEvaluationContext &, uint index) const;
    int getChannelInteger(const StateEvaluationContext &, uint index) const;
    QString getChannelString(const StateEvaluationContext &, uint index) const;
    bool getChannelBool(const StateEvaluationContext &, uint index) const;
    QColor getChannelColor(const StateEvaluationContext &, uint index) const;
    void setChannelValue(uint index, const QVariant &);
    QVariant getChannelValue(uint index) const;
    void resetChannelToDefault(uint index);
    QVector<FixtureCapability*> getFixtureCapabilities(const StateEvaluationContext &) const;

    QByteArray channelId(uint) const;
    uint index() const;
    void setIndex(uint);

    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

protected:
    void addAvailableChannel(const ChannelInfo &info);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_STATECAPABILITY_H

#ifndef PHOTON_WHEELSLOTCAPABILITY_H
#define PHOTON_WHEELSLOTCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

class PHOTONCORE_EXPORT WheelSlotCapability : public FixtureCapability
{
public:
    WheelSlotCapability();
    WheelSlotCapability(DMXRange range, CapabilityType capability = Capability_WheelSlot);
    virtual ~WheelSlotCapability();

    QString wheelName() const;
    int slotNumber() const;
    FixtureWheelSlot *wheelSlot() const;
    void selectSlot(DMXMatrix &t_matrix) const;
    virtual void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_WHEELSLOTCAPABILITY_H

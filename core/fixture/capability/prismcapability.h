#ifndef PHOTON_PRISMCAPABILITY_H
#define PHOTON_PRISMCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

// A prism inserted into the beam: splits it into `facetCount()` copies. Active when
// the channel value is within this capability's DMX range (isValid). The facet count
// is taken from the OpenFixture comment (e.g. "8-facet prism"); linear prisms fan
// their copies in a line rather than a ring.
class PHOTONCORE_EXPORT PrismCapability : public FixtureCapability
{
public:
    PrismCapability();
    ~PrismCapability();

    int facetCount() const;
    bool isLinear() const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PRISMCAPABILITY_H

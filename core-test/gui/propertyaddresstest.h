#ifndef PHOTON_PROPERTYADDRESSTEST_H
#define PHOTON_PROPERTYADDRESSTEST_H

#include <QObject>

namespace photon {

// Covers the parts of the Properties panel consolidation that can be checked
// without a running UI: PropertyAddress as a value type, and the round-trip
// through JSON that pinned tabs depend on when a project is reopened.
//
// The panel, tab strip and address bar are interactive and left to manual
// verification, consistent with this project's existing test scope.
class PropertyAddressTest : public QObject
{
    Q_OBJECT
public:
    explicit PropertyAddressTest(QObject *parent = nullptr);

private slots:
    void emptyAddressHasNoSegments();
    void segmentsPreserveOrder();
    void toStringJoinsIds();
    void toDisplayStringJoinsLabels();
    void jsonRoundTripPreservesSegments();
    void equalityIgnoresLabels();
    void equalityDistinguishesIdsAndKinds();
    void lastReturnsLeafSegment();
};

} // namespace photon

#endif // PHOTON_PROPERTYADDRESSTEST_H

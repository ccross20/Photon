#ifndef FIXTURELISTSORTNODE_H
#define FIXTURELISTSORTNODE_H

#include "model/node.h"
#include "model/parameter/optionparameter.h"
#include "photon-global.h"

namespace photon {

// Reorders a fixture list by each fixture's time offset (e.g. as set by a
// falloff node) - ascending puts the earliest first. Fixtures with equal
// offsets keep their incoming order. Offsets are carried through unchanged,
// so downstream nodes that use list position (index-based effects, Interval
// Subset) follow the timing instead of the original selection order.
class FixtureListSortNode : public keira::Node
{
public:
    enum Order
    {
        OrderAscending,
        OrderDescending,
    };

    const static QByteArray FixturesInParam;
    const static QByteArray OrderParam;
    const static QByteArray FixturesOutParam;

    FixtureListSortNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam;
    keira::OptionParameter *m_orderParam;
    FixtureListParameter *m_outParam;
};

} // namespace photon

#endif // FIXTURELISTSORTNODE_H

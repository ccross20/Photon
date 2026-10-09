#ifndef FIXTURELISTSUBSETNODE_H
#define FIXTURELISTSUBSETNODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "photon-global.h"

namespace photon {

// A contiguous run of Size fixtures starting at Offset, wrapping past the end
// of the list back to its start - the fixture-list twin of Pixel Subset. Both
// values loop: Offset is taken modulo the fixture count (negative counts back
// from the end), and Size cycles 1..count (count + 1 is 1 again; 0 gives
// nothing) - so either can be driven by an ever-increasing counter. Output
// runs in window order, starting at the Offset fixture; offsets travel with
// their fixtures unchanged.
class FixtureListSubsetNode : public keira::Node
{
public:
    const static QByteArray FixturesInParam;
    const static QByteArray OffsetParam;
    const static QByteArray SizeParam;
    const static QByteArray FixturesOutParam;

    FixtureListSubsetNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam;
    keira::IntegerParameter *m_offsetParam;
    keira::IntegerParameter *m_sizeParam;
    FixtureListParameter *m_outParam;
};

} // namespace photon

#endif // FIXTURELISTSUBSETNODE_H

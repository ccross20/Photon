#ifndef FIXTURELISTRANDOMIZENODE_H
#define FIXTURELISTRANDOMIZENODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "photon-global.h"

namespace photon {

// Shuffles a fixture list into a random order. The order depends only on the
// seed (and the list), so the same seed always gives the same shuffle.
// Offsets travel with their fixtures unchanged.
class FixtureListRandomizeNode : public keira::Node
{
public:
    const static QByteArray FixturesInParam;
    const static QByteArray SeedParam;
    const static QByteArray FixturesOutParam;

    FixtureListRandomizeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam;
    keira::IntegerParameter *m_seedParam;
    FixtureListParameter *m_outParam;
};

} // namespace photon

#endif // FIXTURELISTRANDOMIZENODE_H

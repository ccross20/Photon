#ifndef FIXTURELISTMERGENODE_H
#define FIXTURELISTMERGENODE_H

#include "model/node.h"
#include "model/parameter/optionparameter.h"
#include "photon-global.h"

namespace photon {

// Combines two fixture lists into one: either the union of both, or one with
// the other's fixtures taken out. Fixtures are matched by id, so a list can be
// subtracted from another even when the two came from different upstream
// selections.
class FixtureListMergeNode : public keira::Node
{
public:
    enum Mode
    {
        ModeMerge,
        ModeAWithoutB,
        ModeBWithoutA,
    };

    const static QByteArray FixturesAParam;
    const static QByteArray FixturesBParam;
    const static QByteArray ModeParam;
    const static QByteArray FixturesOutParam;

    FixtureListMergeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    FixtureListParameter *m_aParam;
    FixtureListParameter *m_bParam;
    FixtureListParameter *m_outParam;
    keira::OptionParameter *m_modeParam;
};

} // namespace photon

#endif // FIXTURELISTMERGENODE_H

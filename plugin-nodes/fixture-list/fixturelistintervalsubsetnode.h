#ifndef FIXTURELISTINTERVALSUBSETNODE_H
#define FIXTURELISTINTERVALSUBSETNODE_H
#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "photon-global.h"

namespace photon {



class FixtureListIntervalSubsetNode : public keira::Node
{
public:
    const static QByteArray FixturesInParam;
    const static QByteArray OffsetParam;
    const static QByteArray SelectIntervalParam;
    const static QByteArray SkipIntervalParam;
    const static QByteArray FixturesOutParam;

    FixtureListIntervalSubsetNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam;
    FixtureListParameter *m_outParam;
    keira::IntegerParameter *m_offsetParam;
    keira::IntegerParameter *m_selectParam;
    keira::IntegerParameter *m_skipParam;

};

} // namespace photon

#endif // FIXTURELISTINTERVALSUBSETNODE_H

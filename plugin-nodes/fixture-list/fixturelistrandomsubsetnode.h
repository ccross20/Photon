#ifndef FIXTURELISTRANDOMSUBSETNODE_H
#define FIXTURELISTRANDOMSUBSETNODE_H

#include <QRandomGenerator>
#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "photon-global.h"

namespace photon {



class FixtureListRandomSubsetNode : public keira::Node
{
public:
    const static QByteArray FixturesInParam;
    const static QByteArray OddsParam;
    const static QByteArray SeedParam;
    const static QByteArray FixturesOutParam;

    FixtureListRandomSubsetNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam;
    FixtureListParameter *m_outParam;
    keira::IntegerParameter *m_seedParam;
    keira::DecimalParameter *m_oddsParam;

};

} // namespace photon

#endif // FIXTURELISTRANDOMSUBSETNODE_H

#ifndef FIXTUREGLOBALSNODE_H
#define FIXTUREGLOBALSNODE_H
#include "model/node.h"
#include "photon-global.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "graph/parameter/fixtureparameter.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureGlobalsNode : public keira::Node
{
public:
    const static QByteArray DMXParam;
    const static QByteArray FixtureListParam;
    const static QByteArray FixtureParam;
    const static QByteArray FixtureIndexParam;
    const static QByteArray TimeParam;
    const static QByteArray GlobalTimeParam;
    const static QByteArray TimeOffsetParam;

    FixtureGlobalsNode();

    void createParameters() override;


    static keira::NodeInformation info();

private:
    DMXMatrixParameter *m_dmxParam;
    FixtureListParameter *m_fixtureListParam;
    FixtureParameter *m_fixtureParam;
    keira::IntegerParameter *m_fixtureIndexParam;
    keira::DecimalParameter *m_timeParam;
    keira::DecimalParameter *m_globalTimeParam;
    keira::DecimalParameter *m_timeOffsetParam;


};

} // namespace photon

#endif // FIXTUREGLOBALSNODE_H

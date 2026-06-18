#ifndef PIXELGLOBALSNODE_H
#define PIXELGLOBALSNODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "graph/parameter/fixtureparameter.h"

namespace photon {

class PHOTONCORE_EXPORT PixelGlobalsNode : public keira::Node
{
public:
    const static QByteArray FixtureParam;
    const static QByteArray FixtureIndexParam;
    const static QByteArray PixelIndexParam;
    const static QByteArray PixelTotalParam;
    const static QByteArray PixelGlobalIndexParam;
    const static QByteArray TimeParam;
    const static QByteArray GlobalTimeParam;
    const static QByteArray TimeOffsetParam;

    void createParameters() override;


    static keira::NodeInformation info();
    PixelGlobalsNode();

private:
    FixtureParameter *m_fixtureParam;
    keira::IntegerParameter *m_fixtureIndexParam;
    keira::IntegerParameter *m_pixelIndexParam;
    keira::IntegerParameter *m_pixelGlobalIndexParam;
    keira::IntegerParameter *m_pixelTotalParam;
    keira::DecimalParameter *m_timeParam;
    keira::DecimalParameter *m_globalTimeParam;
    keira::DecimalParameter *m_timeOffsetParam;
};

} // namespace photon

#endif // PIXELGLOBALSNODE_H

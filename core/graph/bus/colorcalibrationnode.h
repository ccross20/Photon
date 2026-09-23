#ifndef PHOTON_COLORCALIBRATIONNODE_H
#define PHOTON_COLORCALIBRATIONNODE_H

#include <QMap>
#include "model/node.h"
#include "fixture/capability/fixturecapability.h"
#include "photon-global.h"

namespace photon {

// Sits in the bus pipeline next to IdentifyFixtureNode. When enabled, stamps
// a raw set of per-LED-channel percentages directly onto one fixture's color
// capabilities - bypassing both the RGB heuristic and any saved calibration -
// so the Color Calibration dialog can preview exactly what a slider drag
// would produce on the real fixture. Driven directly by that dialog rather
// than by manual graph wiring, the same way DMXPatchPanel drives Identify.
class PHOTONCORE_EXPORT ColorCalibrationNode : public keira::Node
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;
    const static QByteArray FixtureParam;
    const static QByteArray EnabledParam;

    ColorCalibrationNode();
    ~ColorCalibrationNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    void setTargetFixture(const QByteArray &fixtureId);
    void setPreviewEnabled(bool enabled);
    bool isPreviewEnabled() const;
    void setChannelPercent(CapabilityType, double percent);
    void clearChannelPercents();

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORCALIBRATIONNODE_H

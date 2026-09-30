#ifndef PHOTON_OUTPUTOVERRIDESNODE_H
#define PHOTON_OUTPUTOVERRIDESNODE_H

#include "model/node.h"
#include "fixture/capability/fixturecapability.h"
#include "photon-global.h"

namespace keira { class Graph; }

namespace photon {

class DMXMatrix;

// Last in the bus pipeline, right before the output node: the app's own
// overrides of the show's DMX, applied in a fixed order so the later ones
// always win.
//  1. Identify - one fixture at full white, driven by the DMX Patch panel.
//  2. Color calibration preview - raw per-LED percentages on one fixture,
//     driven by the Color Calibration dialog.
//  3. Laser control - owns every laser's mode channel: setup value while the
//     Laser Setup dialog is open, output value only when armed, otherwise off
//     with dimmer and page zeroed. Last, so nothing can enable a laser.
// Not removable from the bus graph.
class PHOTONCORE_EXPORT OutputOverridesNode : public keira::Node
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;
    const static QByteArray IdentifyFixtureParam;
    const static QByteArray IdentifyParam;
    const static QByteArray CalibrationFixtureParam;
    const static QByteArray CalibrationPreviewParam;

    OutputOverridesNode();
    ~OutputOverridesNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    bool isRemovable() const override { return false; }

    static keira::NodeInformation info();
    static OutputOverridesNode *find(const keira::Graph *bus);

    void setIdentifiedFixture(const QByteArray &fixtureId);
    void setIdentifyEnabled(bool);
    bool isIdentifyEnabled() const;

    void setCalibrationFixture(const QByteArray &fixtureId);
    void setCalibrationPreviewEnabled(bool);
    bool isCalibrationPreviewEnabled() const;
    void setCalibrationChannelPercent(CapabilityType, double percent);
    void clearCalibrationChannelPercents();

    // The last evaluated frame before laser control, so the visualizer can
    // preview lasers that are disarmed in the real rig. Eval thread only.
    DMXMatrix laserPreviewMatrix() const;

    // Seeds every laser channel with its "unchanged" value at the start of a
    // frame, so channels no state drives don't sit at 0 (-100% on centered
    // channels, 0% cue speed).
    static void writeLaserNeutralValues(DMXMatrix &);
    // Writes one laser's mode channel (and setup profile / safety zeroing)
    // according to its arm and setup flags.
    static void applyLaserControl(const Fixture *, DMXMatrix &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_OUTPUTOVERRIDESNODE_H

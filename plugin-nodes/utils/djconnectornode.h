#ifndef DJCONNECTORNODE_H
#define DJCONNECTORNODE_H
#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

class DJConnectorNode : public keira::Node
{
public:
    DJConnectorNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::DecimalParameter *bpmParam;
    keira::DecimalParameter *beatProgressParam;
    keira::DecimalParameter *beatIntensityParam;
    keira::DecimalParameter *beatAmountParam;
    keira::IntegerParameter *beatParam;
    keira::IntegerParameter *songIdParam;
    keira::BooleanParameter *isPlayingParam;
    keira::BooleanParameter *sequenceExistsParam;

    // Beat-reducer controls: Rate re-grids the beat/progress outputs onto a
    // slower ("/4","/2") or faster ("x2","x4","x8") pulse than the DJ's own
    // beat, replacing the old fixed Beat Progress x2/x4 outputs with one
    // general control; Offset (in beats, may be fractional) shifts where
    // that grid's zero point falls.
    keira::OptionParameter *beatRateParam;
    keira::DecimalParameter *beatOffsetParam;

    // Cache the derived song id so evaluate() only re-hashes when the track
    // actually changes, rather than every frame.
    mutable QString m_lastSongIdentity;
    mutable int m_songId = 0;
};

} // namespace photon

#endif // DJCONNECTORNODE_H

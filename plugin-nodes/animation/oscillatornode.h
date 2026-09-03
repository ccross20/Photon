#ifndef PHOTON_OSCILLATORNODE_H
#define PHOTON_OSCILLATORNODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "photon-global.h"

namespace photon {

// Drives a sine wave from a time input. Speed is the period (seconds per full
// cycle); Min/Max set the output range. At Time 0 the output sits at the
// midpoint and rises first.
class OscillatorNode : public keira::Node
{
public:
    OscillatorNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_timeParam;
    keira::DecimalParameter *m_speedParam;
    keira::DecimalParameter *m_minParam;
    keira::DecimalParameter *m_maxParam;
    keira::DecimalParameter *m_outputParam;
};

} // namespace photon

#endif // PHOTON_OSCILLATORNODE_H

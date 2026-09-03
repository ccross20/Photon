#ifndef PHOTON_TIMENODE_H
#define PHOTON_TIMENODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "photon-global.h"

namespace photon {

// Outputs the current evaluation context's global time (seconds). A plain time
// source to feed Oscillator, Pulse, and anything else that takes a time input.
class TimeNode : public keira::Node
{
public:
    TimeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_timeParam;
};

} // namespace photon

#endif // PHOTON_TIMENODE_H

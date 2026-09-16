#ifndef RETIMENODE_H
#define RETIMENODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

// Remaps a time-like number (typically global time) to run faster, slower,
// backwards, or paused, without discontinuities: the output is integrated
// from the input's own rate of change rather than derived directly from its
// value, so changing Speed changes the rate the output moves at from that
// point on without jumping the value itself. Falls back to the eval
// context's own global time when nothing is wired into Time.
class RetimeNode : public keira::Node
{
public:
    RetimeNode();
    ~RetimeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;

    keira::DecimalParameter *m_timeParam;
    keira::DecimalParameter *m_speedParam;
    keira::DecimalParameter *m_outputParam;
};

} // namespace photon

#endif // RETIMENODE_H

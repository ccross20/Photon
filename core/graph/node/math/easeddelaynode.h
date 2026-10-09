#ifndef PHOTON_EASEDDELAYNODE_H
#define PHOTON_EASEDDELAYNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Eases a value between 0 and 1 over Duration seconds whenever a boolean
// changes: Value rises toward 1 while Input is true and falls toward 0 while
// it's false, shaped by Ease. Delayed follows Input once the value has fully
// arrived.
//
// Internally a linear progress moves toward the target at 1/Duration per
// second and Value is that progress through the ease, so an Input that flips
// back mid-way just turns the progress around where it is - no jump - and
// Delayed, never having arrived, doesn't change at all.
class PHOTONCORE_EXPORT EasedDelayNode : public keira::Node
{
public:
    EasedDelayNode();
    ~EasedDelayNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_EASEDDELAYNODE_H

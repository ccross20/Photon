#ifndef PHOTON_ROUNDNODE_H
#define PHOTON_ROUNDNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Rounds a number to a whole value. Mode selects nearest (Round), always-down
// (Floor) or always-up (Ceiling).
class PHOTONCORE_EXPORT RoundNode : public keira::Node
{
public:
    enum Mode { ModeRound = 0, ModeFloor = 1, ModeCeiling = 2 };

    const static QByteArray Input;
    const static QByteArray ModeInput;
    const static QByteArray Output;

    RoundNode();
    ~RoundNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ROUNDNODE_H

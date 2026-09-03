#ifndef PHOTON_MINMAXNODE_H
#define PHOTON_MINMAXNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs the smaller (Min) or larger (Max) of the two inputs.
class PHOTONCORE_EXPORT MinMaxNode : public keira::Node
{
public:
    enum Mode { ModeMin = 0, ModeMax = 1 };

    const static QByteArray InputA;
    const static QByteArray InputB;
    const static QByteArray ModeInput;
    const static QByteArray Output;

    MinMaxNode();
    ~MinMaxNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MINMAXNODE_H

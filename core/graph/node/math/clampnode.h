#ifndef PHOTON_CLAMPNODE_H
#define PHOTON_CLAMPNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Constrains a value to the [Min, Max] range. If Min > Max the two are swapped
// so the node still produces a sane result.
class PHOTONCORE_EXPORT ClampNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray MinInput;
    const static QByteArray MaxInput;
    const static QByteArray Output;

    ClampNode();
    ~ClampNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CLAMPNODE_H

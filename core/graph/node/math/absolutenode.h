#ifndef PHOTON_ABSOLUTENODE_H
#define PHOTON_ABSOLUTENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs the absolute value (magnitude) of the input.
class PHOTONCORE_EXPORT AbsoluteNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Output;

    AbsoluteNode();
    ~AbsoluteNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ABSOLUTENODE_H

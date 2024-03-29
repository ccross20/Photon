#ifndef PHOTON_FIXTUREINFONODE_H
#define PHOTON_FIXTUREINFONODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureInfoNode : public keira::Node
{
public:
    const static QByteArray FixtureInput;
    const static QByteArray PositionOutput;
    const static QByteArray RotationOutput;

    FixtureInfoNode();
    ~FixtureInfoNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREINFONODE_H

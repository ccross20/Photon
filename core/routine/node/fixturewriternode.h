#ifndef PHOTON_FIXTUREWRITERNODE_H
#define PHOTON_FIXTUREWRITERNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureWriterNode : public keira::Node
{
public:
    const static QByteArray BrightnessInput;
    const static QByteArray ColorInput;
    const static QByteArray TiltInput;


    FixtureWriterNode();
    ~FixtureWriterNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREWRITERNODE_H

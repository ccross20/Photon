#ifndef ALLFIXTURESNODE_H
#define ALLFIXTURESNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT AllFixturesNode : public keira::Node
{
public:
    const static QByteArray FixtureParam;

    AllFixturesNode();
    ~AllFixturesNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    uint loopCount() const override;
    void startLoop() override;
    void endLoop() override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // ALLFIXTURESNODE_H

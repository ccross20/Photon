#ifndef PHOTON_SAVEDGRADIENTNODE_H
#define PHOTON_SAVEDGRADIENTNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs a named, project-level saved gradient (managed in the project
// panel's "Gradients" section), resolved live each evaluation - same
// pattern as FixtureGroupNode, just for the Gradients library.
class PHOTONCORE_EXPORT SavedGradientNode : public keira::Node
{
public:
    const static QByteArray GradientParam;
    const static QByteArray ResultParam;

    SavedGradientNode();
    ~SavedGradientNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SAVEDGRADIENTNODE_H

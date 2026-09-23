#ifndef PHOTON_SAVEDCOLORNODE_H
#define PHOTON_SAVEDCOLORNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs a named, project-level saved colour (managed in the project panel's
// "Colors" section), resolved live each evaluation - same pattern as
// FixtureGroupNode, just for the Colors library instead of fixture groups.
class PHOTONCORE_EXPORT SavedColorNode : public keira::Node
{
public:
    const static QByteArray ColorParam;
    const static QByteArray ResultParam;

    SavedColorNode();
    ~SavedColorNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SAVEDCOLORNODE_H

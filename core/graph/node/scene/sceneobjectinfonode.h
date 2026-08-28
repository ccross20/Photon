#ifndef PHOTON_SCENEOBJECTINFONODE_H
#define PHOTON_SCENEOBJECTINFONODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Exposes any scene object's global transform (position + rotation) as a
// single Matrix output - the generic counterpart to FixtureInfoNode, which
// only works for fixtures. Unlike FixtureInfoNode's fixture input (meant to
// be wired from an upstream per-fixture iteration node, with a "current
// fixture" fallback from the routine context), there's no equivalent
// "current object" for a generic scene object, so the object here is always
// picked directly from a dropdown of every object in the project.
class PHOTONCORE_EXPORT SceneObjectInfoNode : public keira::Node
{
public:
    const static QByteArray ObjectParam;
    const static QByteArray MatrixOutput;

    SceneObjectInfoNode();
    ~SceneObjectInfoNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEOBJECTINFONODE_H

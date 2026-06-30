#ifndef FIXTUREGROUPNODE_H
#define FIXTUREGROUPNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs the fixtures of a named, project-level group (a saved FixtureQuery),
// resolved live each evaluation. Lets many shows target the same reusable selection
// by name and update in one place.
class PHOTONCORE_EXPORT FixtureGroupNode : public keira::Node
{
public:
    const static QByteArray GroupParam;
    const static QByteArray ResultParam;

    FixtureGroupNode();
    ~FixtureGroupNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // FIXTUREGROUPNODE_H

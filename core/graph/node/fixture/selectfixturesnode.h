#ifndef SELECTFIXTURESNODE_H
#define SELECTFIXTURESNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Dynamic fixture selection: queries the project's fixtures by tag and type each
// evaluation and outputs the matching set. Because it re-queries live, shows target
// "all movers tagged front" rather than a fixed list — so changing the rig (adding /
// retagging fixtures) updates the selection without editing the show.
class PHOTONCORE_EXPORT SelectFixturesNode : public keira::Node
{
public:
    const static QByteArray TagsParam;
    const static QByteArray TypeParam;
    const static QByteArray ZoneParam;
    const static QByteArray ResultParam;

    SelectFixturesNode();
    ~SelectFixturesNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SELECTFIXTURESNODE_H

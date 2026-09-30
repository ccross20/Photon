#ifndef PHOTON_VIRTUALDJPLAYERNODE_H
#define PHOTON_VIRTUALDJPLAYERNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Plays the Song Library sequence matching whatever VirtualDJ is playing, in
// step with it (see VirtualDJPlayback). Passes DMX through untouched when
// disabled, when nothing matches, or when VirtualDJ goes quiet.
class PHOTONCORE_EXPORT VirtualDJPlayerNode : public keira::Node
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;
    const static QByteArray EnabledParam;
    const static QByteArray OffsetParam;

    VirtualDJPlayerNode();
    ~VirtualDJPlayerNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_VIRTUALDJPLAYERNODE_H

#ifndef PHOTON_GLOBALSNODE_H
#define PHOTON_GLOBALSNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT GlobalsNode : public keira::Node
{
public:
    const static QByteArray RelativeTime;
    const static QByteArray GlobalTime;
    const static QByteArray DelayTime;
    const static QByteArray Strength;
    const static QByteArray Fixture;
    const static QByteArray Index;


    GlobalsNode();
    ~GlobalsNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_GLOBALSNODE_H

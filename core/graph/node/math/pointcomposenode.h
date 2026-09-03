#ifndef PHOTON_POINTCOMPOSENODE_H
#define PHOTON_POINTCOMPOSENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Combines two numbers into a Point (see Point2DParameter). The inverse of
// PointDecomposeNode.
class PHOTONCORE_EXPORT PointComposeNode : public keira::Node
{
public:
    const static QByteArray XInput;
    const static QByteArray YInput;
    const static QByteArray PointOutput;

    PointComposeNode();
    ~PointComposeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_POINTCOMPOSENODE_H

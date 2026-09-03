#ifndef PHOTON_POINTDECOMPOSENODE_H
#define PHOTON_POINTDECOMPOSENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Splits a Point (see Point2DParameter) into its X and Y numbers. The inverse
// of PointComposeNode.
class PHOTONCORE_EXPORT PointDecomposeNode : public keira::Node
{
public:
    const static QByteArray PointInput;
    const static QByteArray XOutput;
    const static QByteArray YOutput;

    PointDecomposeNode();
    ~PointDecomposeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_POINTDECOMPOSENODE_H

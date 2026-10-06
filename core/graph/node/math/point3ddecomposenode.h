#ifndef PHOTON_POINT3DDECOMPOSENODE_H
#define PHOTON_POINT3DDECOMPOSENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Splits a 3D point (see Vector3DParameter) into its X, Y and Z numbers. The
// inverse of Point3DComposeNode.
class PHOTONCORE_EXPORT Point3DDecomposeNode : public keira::Node
{
public:
    const static QByteArray PointInput;
    const static QByteArray XOutput;
    const static QByteArray YOutput;
    const static QByteArray ZOutput;

    Point3DDecomposeNode();
    ~Point3DDecomposeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_POINT3DDECOMPOSENODE_H

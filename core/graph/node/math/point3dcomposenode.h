#ifndef PHOTON_POINT3DCOMPOSENODE_H
#define PHOTON_POINT3DCOMPOSENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Combines three numbers into a 3D point (see Vector3DParameter) - e.g. a
// scene position for Look At Target. The inverse of Point3DDecomposeNode.
class PHOTONCORE_EXPORT Point3DComposeNode : public keira::Node
{
public:
    const static QByteArray XInput;
    const static QByteArray YInput;
    const static QByteArray ZInput;
    const static QByteArray PointOutput;

    Point3DComposeNode();
    ~Point3DComposeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_POINT3DCOMPOSENODE_H

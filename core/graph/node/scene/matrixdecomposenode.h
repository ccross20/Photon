#ifndef PHOTON_MATRIXDECOMPOSENODE_H
#define PHOTON_MATRIXDECOMPOSENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Splits a Matrix (see MatrixParameter) back into Position/Rotation/Scale,
// for whenever a downstream node needs the individual parts rather than the
// combined transform. Note: today nothing in SceneObject ever applies a
// scale when building its matrix, so Scale will read back (1,1,1) for any
// matrix sourced from SceneObjectInfoNode specifically - this node is still
// generically useful for any other matrix producer/math.
class PHOTONCORE_EXPORT MatrixDecomposeNode : public keira::Node
{
public:
    const static QByteArray MatrixInput;
    const static QByteArray PositionOutput;
    const static QByteArray RotationOutput;
    const static QByteArray ScaleOutput;

    MatrixDecomposeNode();
    ~MatrixDecomposeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MATRIXDECOMPOSENODE_H

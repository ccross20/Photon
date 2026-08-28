#include <QQuaternion>
#include <QMatrix3x3>
#include "matrixdecomposenode.h"
#include "graph/parameter/matrixparameter.h"
#include "graph/parameter/vector3dparameter.h"

namespace photon {

const QByteArray MatrixDecomposeNode::MatrixInput = "matrixInput";
const QByteArray MatrixDecomposeNode::PositionOutput = "positionOutput";
const QByteArray MatrixDecomposeNode::RotationOutput = "rotationOutput";
const QByteArray MatrixDecomposeNode::ScaleOutput = "scaleOutput";

class MatrixDecomposeNode::Impl
{
public:
    MatrixParameter *matrixParam;
    Vector3DParameter *positionParam;
    Vector3DParameter *rotationParam;
    Vector3DParameter *scaleParam;
};

keira::NodeInformation MatrixDecomposeNode::info()
{
    keira::NodeInformation toReturn([](){return new MatrixDecomposeNode;});
    toReturn.name = "Matrix Decompose";
    toReturn.nodeId = "photon.graph.matrix-decompose";
    toReturn.categories = {"Scene"};

    return toReturn;
}

MatrixDecomposeNode::MatrixDecomposeNode() : keira::Node("photon.graph.matrix-decompose"),m_impl(new Impl)
{
    setName("Matrix Decompose");
}

MatrixDecomposeNode::~MatrixDecomposeNode()
{
    delete m_impl;
}

void MatrixDecomposeNode::createParameters()
{
    m_impl->matrixParam = new MatrixParameter(MatrixInput, "Matrix", QMatrix4x4{});
    addParameter(m_impl->matrixParam);

    m_impl->positionParam = new Vector3DParameter(PositionOutput, "Position", QVector3D{}, keira::AllowMultipleOutput);
    addParameter(m_impl->positionParam);
    m_impl->rotationParam = new Vector3DParameter(RotationOutput, "Rotation", QVector3D{}, keira::AllowMultipleOutput);
    addParameter(m_impl->rotationParam);
    m_impl->scaleParam = new Vector3DParameter(ScaleOutput, "Scale", QVector3D(1, 1, 1), keira::AllowMultipleOutput);
    addParameter(m_impl->scaleParam);
}

void MatrixDecomposeNode::evaluate(keira::EvaluationContext *) const
{
    const QMatrix4x4 m = m_impl->matrixParam->value().value<QMatrix4x4>();

    m_impl->positionParam->setValue(m.map(QVector3D{}));

    // Same basis-column decomposition SceneObject::globalRotation() uses:
    // the columns' lengths are the scale, and normalizing them out before
    // building the rotation matrix strips that scale from the Euler result.
    QVector3D c0(m(0, 0), m(1, 0), m(2, 0));
    QVector3D c1(m(0, 1), m(1, 1), m(2, 1));
    QVector3D c2(m(0, 2), m(1, 2), m(2, 2));
    const QVector3D scale(c0.length(), c1.length(), c2.length());
    m_impl->scaleParam->setValue(scale);

    // Guard against a degenerate (zero-length) column before normalizing -
    // otherwise it turns into a NaN axis and poisons the whole rotation.
    c0 = c0.length() > 1e-6f ? c0.normalized() : QVector3D(1, 0, 0);
    c1 = c1.length() > 1e-6f ? c1.normalized() : QVector3D(0, 1, 0);
    c2 = c2.length() > 1e-6f ? c2.normalized() : QVector3D(0, 0, 1);
    const float v[9] = {
        c0.x(), c1.x(), c2.x(),
        c0.y(), c1.y(), c2.y(),
        c0.z(), c1.z(), c2.z()
    };
    m_impl->rotationParam->setValue(QQuaternion::fromRotationMatrix(QMatrix3x3(v)).toEulerAngles());
}

} // namespace photon

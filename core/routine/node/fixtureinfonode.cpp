#include "fixtureinfonode.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "graph/parameter/vector3dparameter.h"
#include "graph/parameter/matrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"

namespace photon {

const QByteArray FixtureInfoNode::FixtureInput = "fixtureInput";
const QByteArray FixtureInfoNode::PositionOutput = "positionOutput";
const QByteArray FixtureInfoNode::RotationOutput = "rotationOutput";
const QByteArray FixtureInfoNode::MatrixOutput = "matrixOutput";

class FixtureInfoNode::Impl
{
public:
    FixtureParameter *fixtureParam;
    Vector3DParameter *positionParam;
    Vector3DParameter *rotationParam;
    MatrixParameter *matrixParam;
};


keira::NodeInformation FixtureInfoNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureInfoNode;});
    toReturn.name = "Fixture Info";
    toReturn.nodeId = "photon.routine.fixture-info";

    return toReturn;
}

FixtureInfoNode::FixtureInfoNode() : keira::Node("photon.routine.fixture-info"),m_impl(new Impl)
{
    setName("Fixture Info");
}

FixtureInfoNode::~FixtureInfoNode()
{
    delete m_impl;
}

void FixtureInfoNode::createParameters()
{
    m_impl->fixtureParam = new FixtureParameter(FixtureInput,"Fixture", "");
    addParameter(m_impl->fixtureParam);

    m_impl->positionParam = new Vector3DParameter(PositionOutput,"Position",QVector3D{}, keira::AllowMultipleOutput);
    addParameter(m_impl->positionParam);
    m_impl->rotationParam = new Vector3DParameter(RotationOutput,"Rotation",QVector3D{}, keira::AllowMultipleOutput);
    addParameter(m_impl->rotationParam);

    m_impl->matrixParam = new MatrixParameter(MatrixOutput,"Matrix",QMatrix4x4{}, keira::AllowMultipleOutput);
    addParameter(m_impl->matrixParam);
}

void FixtureInfoNode::evaluate(keira::EvaluationContext *t_context) const
{
    Fixture *fixture = static_cast<RoutineEvaluationContext*>(t_context)->fixture;

    QByteArray fixtureId = m_impl->fixtureParam->value().toByteArray();
    if(!fixtureId.isEmpty())
        fixture = FixtureCollection::fixtureById(fixtureId);

    if(!fixture)
        return;

    m_impl->positionParam->setValue(fixture->globalPosition());
    m_impl->rotationParam->setValue(fixture->globalRotation());

    // The Matrix output is what aim-solving nodes (Look At Target, Look In
    // Direction) use as the fixture's "zero" reference frame to compute the
    // pan needed to hit a target - but Set Fixture Pan then adds panOffset
    // on top of whatever pan value it's given (see AngleCapability::
    // writePercent), before that reaches DMX. Left uncorrected, a solved
    // node's answer - already the true physical angle - gets panOffset
    // added a second time, so the fixture visibly over/under-aims by
    // exactly the offset. Pre-rotating the frame here by panOffset around
    // the (legacy-rig-convention) pan axis exactly cancels that out
    // regardless of tilt, since pan is the outermost rotation in the
    // pan-then-tilt chain.
    //
    // tiltOffset does NOT get the same treatment: tilt is the innermost
    // rotation (applied in the already-panned frame), so a fixed correction
    // to this frame can't compensate it exactly once pan is nonzero. Tilt
    // Offset still works correctly for Set Fixture Tilt driven directly
    // (cues, manual values) - just not through an aim-solving node combined
    // with a nonzero pan.
    QMatrix4x4 aimMatrix = fixture->globalMatrix();
    aimMatrix.rotate(fixture->panOffset(), 0.0f, 1.0f, 0.0f);
    m_impl->matrixParam->setValue(QVariant::fromValue(aimMatrix));
}

} // namespace photon

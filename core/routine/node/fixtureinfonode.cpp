#include "fixtureinfonode.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "graph/parameter/vector3dparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"

namespace photon {

const QByteArray FixtureInfoNode::FixtureInput = "fixtureInput";
const QByteArray FixtureInfoNode::PositionOutput = "positionOutput";
const QByteArray FixtureInfoNode::RotationOutput = "rotationOutput";

class FixtureInfoNode::Impl
{
public:
    FixtureParameter *fixtureParam;
    Vector3DParameter *positionParam;
    Vector3DParameter *rotationParam;
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

    m_impl->positionParam = new Vector3DParameter(PositionOutput,"Position",QVector3D{});
    addParameter(m_impl->positionParam);
    m_impl->rotationParam = new Vector3DParameter(RotationOutput,"Rotation",QVector3D{});
    addParameter(m_impl->rotationParam);
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
    m_impl->rotationParam->setValue(fixture->rotation());
}

} // namespace photon

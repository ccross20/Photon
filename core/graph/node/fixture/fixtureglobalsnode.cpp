#include "fixtureglobalsnode.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"

namespace photon {

const QByteArray FixtureGlobalsNode::DMXParam = "dmxMatrix";
const QByteArray FixtureGlobalsNode::FixtureListParam = "fixtureList";
const QByteArray FixtureGlobalsNode::FixtureParam = "fixture";
const QByteArray FixtureGlobalsNode::FixtureIndexParam = "fixtureIndex";
const QByteArray FixtureGlobalsNode::TimeParam = "relativeTime";
const QByteArray FixtureGlobalsNode::GlobalTimeParam = "globalTime";
const QByteArray FixtureGlobalsNode::TimeOffsetParam = "timeOffset";

keira::NodeInformation FixtureGlobalsNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureGlobalsNode;});
    toReturn.name = "Fixture Globals";
    toReturn.nodeId = "photon.fixture.globals";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixtures"};

    return toReturn;
}

FixtureGlobalsNode::FixtureGlobalsNode()  : keira::Node("photon.fixture.globals"){
    setName("Globals");
    setIsAlwaysDirty(true);
}

void FixtureGlobalsNode::createParameters()
{
    m_dmxParam = new DMXMatrixParameter(DMXParam, "DMX Matrix", DMXMatrix(), keira::AllowMultipleOutput);
    m_fixtureListParam  = new FixtureListParameter(FixtureListParam,"All Fixtures", QVector<FixtureParameterData>()
                                                , keira::AllowMultipleOutput);
    m_fixtureParam = new FixtureParameter(FixtureParam,"Fixture","",keira::AllowMultipleOutput);
    m_timeParam = new keira::DecimalParameter(TimeParam, "Relative Time",0,keira::AllowMultipleOutput);
    m_globalTimeParam = new keira::DecimalParameter(GlobalTimeParam, "Global Time",0,keira::AllowMultipleOutput);
    m_timeOffsetParam = new keira::DecimalParameter(TimeOffsetParam, "Time Offset",0,keira::AllowMultipleOutput);
    m_fixtureIndexParam = new keira::IntegerParameter(FixtureIndexParam, "Fixture Index",0,keira::AllowMultipleOutput);

    addParameter(m_dmxParam);
    addParameter(m_fixtureListParam);
    addParameter(m_fixtureParam);
    addParameter(m_fixtureIndexParam);
    addParameter(m_timeParam);
    addParameter(m_globalTimeParam);
    addParameter(m_timeOffsetParam);

}

void FixtureGlobalsNode::evaluate(keira::EvaluationContext *t_context) const
{
    auto ctx = static_cast<RoutineEvaluationContext*>(t_context);
    m_fixtureParam->setValue(ctx->fixture ? ctx->fixture->uniqueId() : QByteArray{});
    m_fixtureIndexParam->setValue(ctx->fixtureIndex);
    m_timeParam->setValue(ctx->relativeTime);
    m_globalTimeParam->setValue(ctx->globalTime);
    m_timeOffsetParam->setValue(ctx->timeOffset);
    // fixtureList is set once before the fixture loop via setValue — no change needed here
}

} // namespace photon

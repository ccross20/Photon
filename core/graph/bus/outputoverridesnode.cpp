#include <QColor>
#include <QMap>
#include <QMutex>
#include "outputoverridesnode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "model/parameter/booleanparameter.h"
#include "graph/bus/busgraph.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixturechannel.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/dimmercapability.h"
#include "fixture/capability/lasercapability.h"
#include "fixture/capability/shutterstrobecapability.h"
#include "project/project.h"
#include "photoncore.h"

namespace photon {

const QByteArray OutputOverridesNode::InputDMX = "dmxInput";
const QByteArray OutputOverridesNode::OutputDMX = "dmxOutput";
const QByteArray OutputOverridesNode::IdentifyFixtureParam = "identifyFixture";
const QByteArray OutputOverridesNode::IdentifyParam = "identify";
const QByteArray OutputOverridesNode::CalibrationFixtureParam = "calibrationFixture";
const QByteArray OutputOverridesNode::CalibrationPreviewParam = "calibrationPreview";

class OutputOverridesNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    FixtureParameter *identifyFixtureParam;
    keira::BooleanParameter *identifyParam;
    FixtureParameter *calibrationFixtureParam;
    keira::BooleanParameter *calibrationPreviewParam;

    // Written by the calibration dialog (main thread), read on the eval thread.
    mutable QMutex calibrationMutex;
    QMap<CapabilityType, double> calibrationPercents;

    mutable DMXMatrix laserPreview;
};

namespace {

void applyIdentify(const QByteArray &t_fixtureId, DMXMatrix &t_matrix)
{
    Fixture *fixture = FixtureCollection::fixtureById(t_fixtureId);
    if(!fixture)
        return;

    const auto dimmers = fixture->findCapability(Capability_Dimmer);
    if(!dimmers.isEmpty())
        static_cast<DimmerCapability*>(dimmers.first())->setPercent(1.0, t_matrix);

    if(auto *color = fixture->color())
        color->setColor(QColor(Qt::white), t_matrix);

    for(auto *cap : fixture->findCapability(Capability_Strobe))
    {
        auto *shutter = static_cast<ShutterStrobeCapability*>(cap);
        if(shutter->shutterEffect() == ShutterStrobeCapability::Shutter_Open)
        {
            t_matrix.setValue(shutter->channel()->universe() - 1,
                              shutter->channel()->universalChannelNumber(),
                              shutter->range().middle(), 1.0);
            break;
        }
    }
}

QVector<Fixture*> laserFixtures()
{
    QVector<Fixture*> lasers;
    Project *project = photonApp->project();
    if(!project)
        return lasers;

    for(auto *fixture : project->fixtures()->fixtures())
    {
        if(fixture->isLaser())
            lasers.append(fixture);
    }
    return lasers;
}

void writeLaserSetupProfile(const Fixture *t_fixture, DMXMatrix &t_matrix)
{
    using Function = LaserCapability::Function;
    const Fixture::LaserSetup setup = t_fixture->laserSetup();

    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupIntensity))
        cap->setPercent(setup.masterIntensity, t_matrix);
    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupTestFrame))
        cap->setRaw(setup.testFrame, t_matrix);
    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupSizeX))
        cap->setCentered(setup.sizeX, t_matrix);
    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupSizeY))
        cap->setCentered(setup.sizeY, t_matrix);
    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupPositionX))
        cap->setCentered(setup.positionX, t_matrix);
    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupPositionY))
        cap->setCentered(setup.positionY, t_matrix);
    if(auto *cap = LaserCapability::find(t_fixture, Function::Function_SetupRotation))
        cap->setPercent(setup.rotation / 360.0, t_matrix);
}

} // namespace

keira::NodeInformation OutputOverridesNode::info()
{
    keira::NodeInformation toReturn([](){return new OutputOverridesNode;});
    toReturn.name = "Output Overrides";
    toReturn.nodeId = "photon.bus.output-overrides";
    toReturn.graphs = QByteArrayList{BusGraph::BusGraphId};
    return toReturn;
}

OutputOverridesNode *OutputOverridesNode::find(const keira::Graph *t_bus)
{
    if(!t_bus)
        return nullptr;
    for(auto *node : t_bus->nodes())
    {
        if(auto *overrides = dynamic_cast<OutputOverridesNode*>(node))
            return overrides;
    }
    return nullptr;
}

OutputOverridesNode::OutputOverridesNode() : keira::Node("photon.bus.output-overrides"), m_impl(new Impl)
{
    setName("Output Overrides");
    // Laser arm/setup live on the fixtures, not in parameters, so nothing
    // would otherwise mark this node dirty when they change.
    setIsAlwaysDirty(true);
}

OutputOverridesNode::~OutputOverridesNode()
{
    delete m_impl;
}

void OutputOverridesNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX, "DMX Input", DMXMatrix());
    addParameter(m_impl->dmxInParam);

    m_impl->identifyFixtureParam = new FixtureParameter(IdentifyFixtureParam, "Identify Fixture", "");
    addParameter(m_impl->identifyFixtureParam);

    m_impl->identifyParam = new keira::BooleanParameter(IdentifyParam, "Identify", false);
    addParameter(m_impl->identifyParam);

    m_impl->calibrationFixtureParam = new FixtureParameter(CalibrationFixtureParam, "Calibration Fixture", "");
    addParameter(m_impl->calibrationFixtureParam);

    m_impl->calibrationPreviewParam = new keira::BooleanParameter(CalibrationPreviewParam, "Calibration Preview", false);
    addParameter(m_impl->calibrationPreviewParam);

    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX, "DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxOutParam);
}

void OutputOverridesNode::setIdentifiedFixture(const QByteArray &t_fixtureId)
{
    m_impl->identifyFixtureParam->setValue(t_fixtureId);
}

void OutputOverridesNode::setIdentifyEnabled(bool t_enabled)
{
    m_impl->identifyParam->setValue(t_enabled);
}

bool OutputOverridesNode::isIdentifyEnabled() const
{
    return m_impl->identifyParam->value().toBool();
}

void OutputOverridesNode::setCalibrationFixture(const QByteArray &t_fixtureId)
{
    m_impl->calibrationFixtureParam->setValue(t_fixtureId);
}

void OutputOverridesNode::setCalibrationPreviewEnabled(bool t_enabled)
{
    m_impl->calibrationPreviewParam->setValue(t_enabled);
}

bool OutputOverridesNode::isCalibrationPreviewEnabled() const
{
    return m_impl->calibrationPreviewParam->value().toBool();
}

void OutputOverridesNode::setCalibrationChannelPercent(CapabilityType t_type, double t_percent)
{
    QMutexLocker lock(&m_impl->calibrationMutex);
    m_impl->calibrationPercents.insert(t_type, t_percent);
}

void OutputOverridesNode::clearCalibrationChannelPercents()
{
    QMutexLocker lock(&m_impl->calibrationMutex);
    m_impl->calibrationPercents.clear();
}

DMXMatrix OutputOverridesNode::laserPreviewMatrix() const
{
    return m_impl->laserPreview;
}

void OutputOverridesNode::writeLaserNeutralValues(DMXMatrix &t_matrix)
{
    for(auto *fixture : laserFixtures())
        LaserCapability::writeNeutralValues(fixture, t_matrix);
}

void OutputOverridesNode::applyLaserControl(const Fixture *t_fixture, DMXMatrix &t_matrix)
{
    auto *mode = LaserCapability::find(t_fixture, LaserCapability::Function_Mode);

    if(t_fixture->isLaserSetupActive())
    {
        if(mode)
            mode->setRaw(LaserCapability::ModeSetup, t_matrix);
        writeLaserSetupProfile(t_fixture, t_matrix);
    }
    else if(t_fixture->isLaserArmed())
    {
        if(mode)
            mode->setRaw(LaserCapability::ModeOutput, t_matrix);
    }
    else
    {
        if(mode)
            mode->setRaw(LaserCapability::ModeOff, t_matrix);
        for(auto *dimmer : t_fixture->findCapability<DimmerCapability*>())
            dimmer->setPercent(0.0, t_matrix);
        if(auto *page = LaserCapability::find(t_fixture, LaserCapability::Function_Page))
            page->setRaw(0, t_matrix);
    }
}

void OutputOverridesNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();

    if(m_impl->identifyParam->value().toBool())
        applyIdentify(m_impl->identifyFixtureParam->value().toByteArray(), matrix);

    if(m_impl->calibrationPreviewParam->value().toBool())
    {
        if(Fixture *fixture = FixtureCollection::fixtureById(m_impl->calibrationFixtureParam->value().toByteArray()))
        {
            QMap<CapabilityType, double> percents;
            {
                QMutexLocker lock(&m_impl->calibrationMutex);
                percents = m_impl->calibrationPercents;
            }
            for(int i = 0; i < fixture->colorCount(); ++i)
                fixture->colorAtIndex(i)->setChannelPercents(percents, matrix);
        }
    }

    m_impl->laserPreview = matrix;
    for(auto *fixture : laserFixtures())
        applyLaserControl(fixture, matrix);

    m_impl->dmxOutParam->setValue(matrix);
}

} // namespace photon

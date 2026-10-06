#include <algorithm>
#include <cmath>
#include <QMatrix4x4>
#include <QVector3D>
#include "spatialfalloffnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "scene/scenefalloff.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "scene/scenemanager.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "project/project.h"
#include "photoncore.h"

namespace photon {

keira::NodeInformation SpatialFalloffNode::info()
{
    keira::NodeInformation toReturn([](){return new SpatialFalloffNode;});
    toReturn.name = "Spatial Falloff";
    toReturn.nodeId = "photon.falloff.spatial";
    toReturn.categories = {"Falloff"};

    return toReturn;
}

SpatialFalloffNode::SpatialFalloffNode() : keira::Node("photon.falloff.spatial")
{
    setName("Spatial Falloff");
}

void SpatialFalloffNode::createParameters()
{
    m_inParam = new FixtureListParameter("in", "Fixtures In", {});
    addParameter(m_inParam);

    m_helperParam = new keira::StringOptionParameter("helper", "Falloff", {}, 0);
    m_helperParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        options.append({"(none)", QString()});
        if(Project *project = photonApp->project())
        {
            for(SceneObject *object : SceneIterator::ToList(project->sceneRoot()))
            {
                if(object->typeId() == "falloff")
                    options.append({object->name(), QString::fromUtf8(object->uniqueId())});
            }
        }
        return options;
    });
    addParameter(m_helperParam);

    // Appended, never reordered - the stored value is the option index.
    m_modeParam = new keira::OptionParameter("mode", "Mode", {"Bounded", "Unbounded"}, ModeBounded);
    addParameter(m_modeParam);

    m_multiplierParam = new keira::DecimalParameter("multiplier", "Multiplier", 1.0);
    addParameter(m_multiplierParam);

    m_outParam = new FixtureListParameter("out", "Fixtures Out", {}, keira::AllowMultipleOutput);
    addParameter(m_outParam);
}

void SpatialFalloffNode::evaluate(keira::EvaluationContext *) const
{
    auto fixtures = m_inParam->resolvedValue();

    Project *project = photonApp->project();
    const QByteArray helperId = m_helperParam->value().toString().toUtf8();
    SceneObject *object = (project && !helperId.isEmpty())
                              ? project->scene()->findObjectById(helperId)
                              : nullptr;
    auto *helper = dynamic_cast<SceneFalloff *>(object);

    // Nothing to work with - pass the list through untouched rather than
    // stamping every fixture with 0.
    if(!helper || fixtures.isEmpty())
    {
        m_outParam->setValue(QVariant::fromValue(fixtures));
        return;
    }

    const double multiplier = m_multiplierParam->value().toDouble();
    const QVector3D origin = helper->globalPosition();
    auto fixturePosition = [&](const FixtureParameterData &data) {
        Fixture *fix = project->fixtures()->fixtureWithId(data.fixtureId);
        return fix ? fix->globalPosition() : origin;
    };

    if(m_modeParam->value().toInt() == ModeUnbounded)
    {
        // The falloff only gives the shape and direction: its raw values are
        // stretched so the fixtures' own extremes land exactly on 0 and 1.
        QVector<double> raw(fixtures.size());
        for(int i = 0; i < fixtures.size(); ++i)
            raw[i] = helper->rawValueAt(fixturePosition(fixtures[i]));
        const auto [lo, hi] = std::minmax_element(raw.cbegin(), raw.cend());
        const double span = *hi - *lo;
        for(int i = 0; i < fixtures.size(); ++i)
            fixtures[i].offset = (span > 1e-9 ? (raw[i] - *lo) / span : 0.0) * multiplier;
    }
    else
    {
        // Bounded: the falloff's own amount - shape, mirroring and what happens
        // past the end all included.
        for(int i = 0; i < fixtures.size(); ++i)
            fixtures[i].offset = helper->amountAt(fixturePosition(fixtures[i])) * multiplier;
    }

    m_outParam->setValue(QVariant::fromValue(fixtures));
}

} // namespace photon

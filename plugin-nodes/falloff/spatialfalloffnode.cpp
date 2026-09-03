#include <algorithm>
#include <cmath>
#include <QMatrix4x4>
#include <QVector3D>
#include "spatialfalloffnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "scene/scenelinearfalloff.h"
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

    m_helperParam = new keira::StringOptionParameter("helper", "Linear Falloff", {}, 0);
    m_helperParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        options.append({"(none)", QString()});
        if(Project *project = photonApp->project())
        {
            for(SceneObject *object : SceneIterator::ToList(project->sceneRoot()))
            {
                if(object->typeId() == "linearfalloff")
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
    auto *helper = dynamic_cast<SceneLinearFalloff *>(object);

    // Nothing to work with - pass the list through untouched rather than
    // stamping every fixture with 0.
    if(!helper || fixtures.isEmpty())
    {
        m_outParam->setValue(QVariant::fromValue(fixtures));
        return;
    }

    const QMatrix4x4 m = helper->globalMatrix();
    const QVector3D origin = m.map(QVector3D(0, 0, 0));
    // The helper's arrow runs along its local +Y (see appendArrowLines / the
    // scene-helper-objects notes).
    QVector3D axis = m.mapVector(QVector3D(0, 1, 0));
    if(axis.lengthSquared() < 1e-8f)
    {
        m_outParam->setValue(QVariant::fromValue(fixtures));
        return;
    }
    axis.normalize();

    // Signed distance of each fixture along the line from the helper's origin.
    QVector<double> projected(fixtures.size());
    for(int i = 0; i < fixtures.size(); ++i)
    {
        Fixture *fix = project->fixtures()->fixtureWithId(fixtures[i].fixtureId);
        const QVector3D pos = fix ? fix->globalPosition() : origin;
        projected[i] = QVector3D::dotProduct(pos - origin, axis);
    }

    const double multiplier = m_multiplierParam->value().toDouble();

    if(m_modeParam->value().toInt() == ModeUnbounded)
    {
        // Direction only: normalise against the spread of the fixtures
        // themselves, so the extremes land exactly on 0 and 1.
        double lo = projected[0], hi = projected[0];
        for(double v : projected)
        {
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
        const double span = hi - lo;
        for(int i = 0; i < fixtures.size(); ++i)
        {
            const double s = span > 1e-9 ? (projected[i] - lo) / span : 0.0;
            fixtures[i].offset = s * multiplier;
        }
    }
    else
    {
        // Bounded: actual position on the line, 0 at the start bar, 1 at the tip.
        const double length = std::max(1e-6, double(helper->length()));
        for(int i = 0; i < fixtures.size(); ++i)
        {
            const double s = std::clamp(projected[i] / length, 0.0, 1.0);
            fixtures[i].offset = s * multiplier;
        }
    }

    m_outParam->setValue(QVariant::fromValue(fixtures));
}

} // namespace photon

#include <QPointF>
#include <QRandomGenerator>
#include <QVector>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <numeric>

#include "scatterbeams.h"
#include "routine/routineevaluationcontext.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "scene/scenemanager.h"
#include "scene/sceneboundaryrectangle.h"
#include "scene/sceneboundaryoval.h"
#include "util/noisegenerator.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

namespace {

// How fast the noise driving the steering evolves, relative to the beam's own
// travel. Low enough that the turn rate holds roughly steady over a stretch of
// path - which is what produces long sweeping runs rather than a jitter.
constexpr double kWanderRate = 0.35;

// Where the beam starts steering back inward, as a fraction of the way from
// the centre to the rim. Well short of the rim so it has room to come around
// within its turn radius instead of arriving at the edge already committed.
//
// Measured: at this value the beam overshoots the rim by at most ~0.3% of the
// area's half-size before the backstop below catches it, and at tight radii it
// never reaches the rim at all. Scaling this with Curve Radius was tried and
// is worse - it steers too late at small radii and doesn't help at large ones,
// where the turning circle simply won't fit inside the area whatever you do.
constexpr double kEdgeStart = 0.55;

// Per-beam stride through noise space, deliberately not a whole number: Perlin
// noise is zero at every integer lattice point, so an integer stride would sit
// every beam on the same degenerate line.
constexpr double kBeamNoiseStride = 12.9898;

// A time step larger than this means playback jumped (scrubbed, paused, looped)
// rather than advanced a frame. Integrating across it would fling the beam, so
// the step is skipped and the beam simply resumes from where it was.
constexpr double kMaxStep = 0.25;

double wrapToPi(double t_angle)
{
    while(t_angle > M_PI)  t_angle -= 2.0 * M_PI;
    while(t_angle < -M_PI) t_angle += 2.0 * M_PI;
    return t_angle;
}

} // namespace

class ScatterBeams::Impl
{
public:
    NoiseGenerator noise{NoiseGenerator::NoiseMode1D};
    int noiseSeed = 0;
    bool noiseSeeded = false;

    // This beam's live state. Private to one fixture: the fixture subgraph
    // clones the whole node per fixture, so there is no sharing across the
    // parallel evaluations.
    QPointF position;      // normalized shape space, centre origin
    double heading = 0.0;  // radians
    double lastTime = 0.0;
    bool started = false;

    // Deterministic starting point/heading, so a given seed and beam always
    // begin the same way.
    void start(int t_seed, int t_index, int t_count);
};

void ScatterBeams::Impl::start(int t_seed, int t_index, int t_count)
{
    const int count = std::max(1, t_count);
    const int index = qBound(0, t_index, count - 1);

    // Which of the evenly-spaced slots this beam takes. The seed has to reach
    // the POSITION, not just the heading: with Speed at 0 a beam never moves
    // off where it starts, so if the layout ignored the seed then re-seeding
    // would visibly do nothing at all.
    //
    // Shuffling the slot assignment (rather than jittering each beam on its
    // own) is what lets the seed rearrange the group while keeping the spacing
    // even. Every beam runs this same shuffle from the seed alone, so the
    // parallel per-fixture clones agree on who goes where without sharing
    // anything between them.
    QRandomGenerator layoutGen(static_cast<quint32>(t_seed) * 2654435761u);
    // Not named "slots" - that's one of Qt's keyword macros (with signals and
    // emit) and expands to nothing, which turns the declaration into garbage.
    QVector<int> slotOrder(count);
    std::iota(slotOrder.begin(), slotOrder.end(), 0);
    std::shuffle(slotOrder.begin(), slotOrder.end(), layoutGen);
    const int slot = slotOrder.at(index);

    // Turning the whole layout as well, so a re-seed still moves things when
    // there is only one beam and the shuffle has nothing to permute.
    const double spin = layoutGen.generateDouble() * 2.0 * M_PI;

    // Sunflower layout: the sqrt on the radius is what keeps the density even
    // rather than crowding the centre.
    const double goldenAngle = 2.39996322972865332;
    const double radius = std::sqrt((slot + 0.5) / double(count));
    const double angle = slot * goldenAngle + spin;
    position = QPointF(radius * std::cos(angle), radius * std::sin(angle)) * 0.8;

    // Seeded per beam as well as per seed, so they don't all set off in the
    // same direction.
    QRandomGenerator headingGen(static_cast<quint32>(t_seed) * 2654435761u
                                + static_cast<quint32>(index) + 1u);
    heading = headingGen.generateDouble() * 2.0 * M_PI;
    started = true;
}

keira::NodeInformation ScatterBeams::info()
{
    keira::NodeInformation toReturn([](){return new ScatterBeams;});
    toReturn.name = "Scatter Beams";
    toReturn.nodeId = "photon.plugin.node.scatter-beams";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

ScatterBeams::ScatterBeams() : keira::Node("photon.plugin.node.scatter-beams"), m_impl(new Impl)
{
    // Driven by time, so it has to re-evaluate every frame even when no input
    // parameter has changed.
    setIsAlwaysDirty(true);

    m_impl->noise.setNoiseType(NoiseGenerator::Perlin);
    m_impl->noise.setBounds(-1.0, 1.0);
    // 1:1 with the sample coordinates, so kWanderRate means what it says rather
    // than being rescaled by FastNoise's small default frequency.
    m_impl->noise.setFrequency(1.0);
}

ScatterBeams::~ScatterBeams()
{
    delete m_impl;
}

void ScatterBeams::createParameters()
{
    m_areaParam = new keira::StringOptionParameter("area", "Area", {}, 0);
    m_areaParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        options.append(std::pair<QString, QString>("(none)", QString()));
        if(Project *project = photonApp->project())
        {
            for(SceneObject *object : SceneIterator::ToList(project->sceneRoot()))
            {
                // Only the two shapes that define an area to scatter across.
                if(object->typeId() != "boundaryrectangle" && object->typeId() != "boundaryoval")
                    continue;
                // Keyed by uniqueId so renaming the boundary doesn't break the pick.
                options.append(std::pair<QString, QString>(
                    object->name() + " (" + QString::fromUtf8(object->typeId()) + ")",
                    QString::fromUtf8(object->uniqueId())));
            }
        }
        return options;
    });
    addParameter(m_areaParam);

    // Wire from the Globals node's "Total Fixtures" port; the default just keeps
    // an unwired node producing a sensible-looking spread.
    m_countParam = new keira::IntegerParameter("fixtureCount", "Fixture Count", 8);
    m_countParam->setMinimum(1);
    addParameter(m_countParam);

    // Bounded well inside int range rather than left at the default full range:
    // NoiseGenerator::setSeed() derives a second seed as seed+1, which would
    // overflow at INT_MAX, and a scatter has no use for four billion distinct
    // layouts anyway.
    m_seedParam = new keira::IntegerParameter("seed", "Seed", 1);
    m_seedParam->setMinimum(0);
    m_seedParam->setMaximum(999999);
    addParameter(m_seedParam);

    m_speedParam = new keira::DecimalParameter("speed", "Speed", 0.2);
    m_speedParam->setMinimum(0.0);    
    m_speedParam->setMaximum(2.0);
    addParameter(m_speedParam);

    // The tightest turn a beam is allowed to make, as a fraction of the area's
    // half-size. Small values let it curl back on itself; large values force
    // long, lazy bends. This is a hard limit, not an average - see evaluate().
    //
    // Measured: the actual tightest turn matches this exactly, while the
    // typical turn runs several times wider and varies about tenfold along the
    // path, which is what gives the meander. Past ~0.6 the turning circle
    // approaches the size of the area itself, so the beam ends up riding the
    // boundary at a near-constant curvature instead of wandering - hence the
    // default well below that.
    m_curveRadiusParam = new keira::DecimalParameter("curveRadius", "Curve Radius", 0.3);
    m_curveRadiusParam->setMinimum(0.02);
    m_curveRadiusParam->setMaximum(1.0);
    addParameter(m_curveRadiusParam);

    m_targetParam = new MatrixParameter("targetOutput", "Target Location", QMatrix4x4{}, keira::AllowMultipleOutput);
    addParameter(m_targetParam);
}

void ScatterBeams::evaluate(keira::EvaluationContext *t_context) const
{
    const QByteArray areaId = m_areaParam->value().toString().toUtf8();
    if(areaId.isEmpty() || !photonApp->project())
        return;

    SceneObject *object = photonApp->project()->scene()->findObjectById(areaId);
    if(!object)
        return;

    float width = 0.0f;
    float height = 0.0f;
    bool isOval = false;
    if(auto *rect = dynamic_cast<SceneBoundaryRectangle*>(object))
    {
        width = rect->width();
        height = rect->height();
    }
    else if(auto *oval = dynamic_cast<SceneBoundaryOval*>(object))
    {
        width = oval->width();
        height = oval->height();
        isOval = true;
    }
    else
        return;   // the picked object isn't a boundary shape

    // Which fixture of the group this evaluation is for. A fixture graph runs
    // this node once per fixture with fixtureIndex set; anywhere else (e.g. a
    // graph being live-ticked in the editor) fall back to the first beam.
    int index = 0;
    double time = 0.0;
    if(auto *context = dynamic_cast<RoutineEvaluationContext*>(t_context))
    {
        index = context->fixtureIndex;
        // globalTime, not the FrameTime in the base context: that one is only
        // populated on the live-bus path and stays zeroed during clip playback.
        time = context->globalTime;
    }

    const int seed = m_seedParam->value().toInt();
    const int count = std::max(1, m_countParam->value().toInt());
    // Keeps the layout coherent if Fixture Count is left unwired or is stale
    // relative to the actual group size.
    index = ((index % count) + count) % count;

    if(m_impl->noiseSeed != seed || !m_impl->noiseSeeded)
    {
        m_impl->noise.setSeed(seed);
        m_impl->noiseSeed = seed;
        m_impl->noiseSeeded = true;
        m_impl->started = false;   // re-seeding restarts the group's layout
    }

    if(!m_impl->started)
    {
        m_impl->start(seed, index, count);
        m_impl->lastTime = time;
    }

    const double speed = m_speedParam->value().toDouble();
    const double radius = qBound(0.02, m_curveRadiusParam->value().toDouble(), 1.0);

    const double step = time - m_impl->lastTime;
    m_impl->lastTime = time;

    // Only integrate across a genuine forward frame step. A negative or
    // oversized step means playback moved rather than advanced, so the beam
    // holds position and picks up again from there.
    if(step > 0.0 && step <= kMaxStep && speed > 0.0)
    {
        // Curvature is the turn per unit of distance travelled, so capping the
        // turn rate at speed/radius caps curvature at 1/radius - i.e. the beam
        // can never carve an arc tighter than `radius`. That is what makes the
        // parameter a real limit rather than an approximation.
        const double maxTurnRate = speed / radius;

        // Smooth noise in [-1,1] steers the beam. Because it varies gradually,
        // the beam holds a near-constant curvature for a stretch at a time -
        // straight-ish where the noise sits near zero, curling where it peaks -
        // which is what reads as meandering rather than orbiting.
        double turn = m_impl->noise.noise1D(index * kBeamNoiseStride, time * kWanderRate);

        // Distance out from the centre, measured in the shape's own terms so a
        // rectangle can use its corners and an ellipse stays round.
        const double edge = isOval
            ? std::hypot(m_impl->position.x(), m_impl->position.y())
            : std::max(std::fabs(m_impl->position.x()), std::fabs(m_impl->position.y()));

        if(edge > kEdgeStart)
        {
            // Blend the wander toward "turn inward" as the rim approaches. The
            // result still passes through the same turn-rate cap below, so
            // steering back in bends the path rather than kinking it.
            const double weight = qBound(0.0, (edge - kEdgeStart) / (1.0 - kEdgeStart), 1.0);
            const double inward = std::atan2(-m_impl->position.y(), -m_impl->position.x());
            const double toInward = wrapToPi(inward - m_impl->heading);
            const double steer = toInward >= 0.0 ? 1.0 : -1.0;
            turn = turn * (1.0 - weight) + steer * weight;
        }

        m_impl->heading += qBound(-1.0, turn, 1.0) * maxTurnRate * step;
        m_impl->position += QPointF(std::cos(m_impl->heading), std::sin(m_impl->heading)) * speed * step;

        // Backstop. Steering keeps the beam inside on its own for any radius
        // that can actually turn within the area, but a large Curve Radius asks
        // for a turning circle wider than the area itself, and then no amount
        // of steering can hold it. Clamping keeps the beam on the boundary in
        // that case instead of wandering off the floor.
        if(isOval)
        {
            const double length = std::hypot(m_impl->position.x(), m_impl->position.y());
            if(length > 1.0)
                m_impl->position /= length;
        }
        else
        {
            m_impl->position.setX(qBound(-1.0, m_impl->position.x(), 1.0));
            m_impl->position.setY(qBound(-1.0, m_impl->position.y(), 1.0));
        }
    }

    // Boundary shapes lie in their local XY plane, centred on the origin, with
    // width()/height() as full extents.
    const QVector3D local(float(m_impl->position.x()) * width * 0.5f,
                          float(m_impl->position.y()) * height * 0.5f, 0.0f);
    const QVector3D world = object->globalMatrix().map(local);

    // Look At Target reads only the translation out of its Target Location.
    QMatrix4x4 result;
    result.translate(world);
    m_targetParam->setValue(QVariant::fromValue(result));
}

} // namespace photon

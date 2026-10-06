#ifndef ROUTINEEVALUATIONCONTEXT_H
#define ROUTINEEVALUATIONCONTEXT_H

#include <functional>
#include <QHash>
#include <QSize>
#include "photon-global.h"
#include "data/dmxmatrix.h"
#include "model/node.h"

class QRhiCommandBuffer;
namespace photon {

struct RoutineEvaluationContext : keira::EvaluationContext
{
    RoutineEvaluationContext(DMXMatrix &matrix) : dmxMatrix(matrix) {}

    // Reference member prevents compiler-generated copy — provide one explicitly.
    // The copy shares the same DMXMatrix, which is safe because parallel fixture
    // evaluations write to non-overlapping channels.
    RoutineEvaluationContext(const RoutineEvaluationContext &o)
        : keira::EvaluationContext(o), dmxMatrix(o.dmxMatrix)
    {
        project      = o.project;
        fixture      = o.fixture;
        surface      = o.surface;
        canvas       = o.canvas;
        frame        = o.frame;
        relativeTime = o.relativeTime;
        globalTime   = o.globalTime;
        delayTime    = o.delayTime;
        strength     = o.strength;
        fixtureIndex = o.fixtureIndex;
        timeOffset   = o.timeOffset;
        clipTime     = o.clipTime;
        clipStrengthAt = o.clipStrengthAt;
        gizmoValues  = o.gizmoValues;
        rhiContext        = o.rhiContext;
        rhiCommandBuffer  = o.rhiCommandBuffer;
        canvasResolution  = o.canvasResolution;
    }
    DMXMatrix &dmxMatrix;
    Project *project = nullptr;
    Fixture *fixture = nullptr;
    Surface *surface = nullptr;
    Canvas *canvas = nullptr;
    qlonglong frame = 0;
    // Inside a sequence, globalTime is song time and relativeTime is time
    // within the clip being evaluated. In a bus or surface graph (no song)
    // both are the running time. A Fixture Graph delays relativeTime by each
    // fixture's offset and leaves globalTime as song time.
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
    int fixtureIndex = 0;
    double timeOffset = 0.0;

    // Set while a clip evaluates its graph: the time within the clip, and the
    // clip's strength envelope (strength, ease in/out) at any time within it -
    // so a node that delays fixtures can give each one the envelope at its own
    // delayed time instead of the shared `strength`. Empty outside a clip.
    double clipTime = 0.0;
    std::function<double(double)> clipStrengthAt;
    // Reported back by nodes that delay fixtures: the largest and smallest
    // per-fixture offset applied this frame, so the clip knows how far past
    // its own ends fixtures are still playing.
    double maxFixtureOffset = 0.0;
    double minFixtureOffset = 0.0;

    // GPU canvas graph: the shared offscreen device and the command buffer of the
    // frame the CanvasSubGraphNode has opened. Canvas nodes record their passes on
    // this command buffer and acquire pooled textures from rhiContext. Null outside
    // a canvas subgraph evaluation.
    RhiContext *rhiContext = nullptr;
    QRhiCommandBuffer *rhiCommandBuffer = nullptr;
    QSize canvasResolution;   // the sink resolution canvas producer nodes render at

    // Surface value bus: "<gizmoUniqueId>/<portId>" -> live value, published by
    // SurfaceNode each frame and read by GizmoValueNode.
    QHash<QByteArray, QVariant> gizmoValues;
};

}

#endif // ROUTINEEVALUATIONCONTEXT_H

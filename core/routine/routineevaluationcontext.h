#ifndef ROUTINEEVALUATIONCONTEXT_H
#define ROUTINEEVALUATIONCONTEXT_H

#include <QHash>
#include "photon-global.h"
#include "data/dmxmatrix.h"
#include "model/node.h"

class QOpenGLContext;
namespace photon {
class OpenGLFrameBuffer;

struct RoutineEvaluationContext : keira::EvaluationContext
{
    RoutineEvaluationContext(DMXMatrix &matrix) : dmxMatrix(matrix) {}

    // Reference member prevents compiler-generated copy — provide one explicitly.
    // The copy shares the same DMXMatrix, which is safe because parallel fixture
    // evaluations write to non-overlapping channels.
    RoutineEvaluationContext(const RoutineEvaluationContext &o)
        : keira::EvaluationContext(o), dmxMatrix(o.dmxMatrix)
    {
        timeMachine  = o.timeMachine;
        project      = o.project;
        fixture      = o.fixture;
        surface      = o.surface;
        frameBuffer  = o.frameBuffer;
        openglContext = o.openglContext;
        resources    = o.resources;
        canvas       = o.canvas;
        frame        = o.frame;
        relativeTime = o.relativeTime;
        globalTime   = o.globalTime;
        delayTime    = o.delayTime;
        strength     = o.strength;
        fixtureIndex = o.fixtureIndex;
        timeOffset   = o.timeOffset;
        gizmoValues  = o.gizmoValues;
    }
    DMXMatrix &dmxMatrix;
    DMXTimeMachine *timeMachine = nullptr;
    Project *project = nullptr;
    Fixture *fixture = nullptr;
    Surface *surface = nullptr;
    OpenGLFrameBuffer *frameBuffer = nullptr;
    QOpenGLContext *openglContext = nullptr;
    OpenGLResources *resources = nullptr;
    Canvas *canvas = nullptr;
    qlonglong frame = 0;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
    int fixtureIndex = 0;
    double timeOffset = 0.0;

    // Surface value bus: "<gizmoUniqueId>/<portId>" -> live value, published by
    // SurfaceNode each frame and read by GizmoValueNode.
    QHash<QByteArray, QVariant> gizmoValues;
};

}

#endif // ROUTINEEVALUATIONCONTEXT_H

#ifndef ROUTINEEVALUATIONCONTEXT_H
#define ROUTINEEVALUATIONCONTEXT_H

#include "photon-global.h"
#include "data/dmxmatrix.h"
#include "model/node.h"

class QOpenGLContext;
namespace photon {
class OpenGLFrameBuffer;

struct RoutineEvaluationContext : keira::EvaluationContext
{
    RoutineEvaluationContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    DMXMatrix &dmxMatrix;
    Project *project = nullptr;
    Fixture *fixture = nullptr;
    OpenGLFrameBuffer *frameBuffer = nullptr;
    QOpenGLContext *openglContext = nullptr;
    OpenGLResources *resources = nullptr;
    Canvas *canvas = nullptr;
    QHash<QByteArray,QVariant> channelValues;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
};

}

#endif // ROUTINEEVALUATIONCONTEXT_H

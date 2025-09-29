#ifndef PROCESSCONTEXT_H
#define PROCESSCONTEXT_H


#include "photon-global.h"
#include "data/dmxmatrix.h"

class QOpenGLContext;

namespace photon {

class OpenGLFrameBuffer;

class ProcessContext
{
public:
    ProcessContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    Fixture *fixture = nullptr;
    Project *project = nullptr;
    OpenGLFrameBuffer *frameBuffer = nullptr;
    QHash<QByteArray,QVariant> channelValues;
    Canvas *canvas = nullptr;
    QOpenGLContext *openglContext;
    OpenGLResources *resources = nullptr;
    QImage *image;
    DMXMatrix &dmxMatrix;
    double globalTime;
    double relativeTime;
};

}//end photon namespace

#endif // PROCESSCONTEXT_H

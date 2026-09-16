#ifndef PROCESSCONTEXT_H
#define PROCESSCONTEXT_H


#include <QVector>
#include <QColor>
#include "photon-global.h"
#include "data/dmxmatrix.h"


namespace photon {

class ProcessContext
{
public:
    ProcessContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    Fixture *fixture = nullptr;
    Project *project = nullptr;
    QHash<QByteArray,QVariant> channelValues;
    Canvas *canvas = nullptr;
    QImage *image = nullptr;
    DMXMatrix &dmxMatrix;
    double globalTime;
    double relativeTime;

    // GPU-gather fast path (canvas DMX 5b): when set, PixelSources consume one
    // colour per sample point from here (in the same order collectSampleUVs
    // produced them) instead of sampling `image`. gatheredIndex advances as
    // sources are processed.
    const QVector<QColor> *gatheredColors = nullptr;
    mutable int gatheredIndex = 0;
};

}//end photon namespace

#endif // PROCESSCONTEXT_H

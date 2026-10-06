#ifndef SURFACEGRAPH_H
#define SURFACEGRAPH_H
#include "model/graph.h"
#include "photon-global.h"

#include "photon-global.h"
#include "data/dmxmatrix.h"
#include "model/node.h"

namespace photon {

struct SurfaceEvaluationContext : keira::EvaluationContext
{
    SurfaceEvaluationContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    DMXMatrix &dmxMatrix;
    Project *project = nullptr;
    Fixture *fixture = nullptr;
    Canvas *canvas = nullptr;
    qlonglong frame = 0;
    QHash<QByteArray,QVariant> channelValues;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
};




class PHOTONCORE_EXPORT SurfaceGraph : public keira::Graph
{
public:
    SurfaceGraph(Surface *);

    Surface *surface() const {return m_surface;}

private:
    Surface *m_surface;
};

} // namespace photon

#endif // SURFACEGRAPH_H

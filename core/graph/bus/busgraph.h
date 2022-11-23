#ifndef PHOTON_BUSGRAPH_H
#define PHOTON_BUSGRAPH_H
#include "model/graph.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT BusGraph : public keira::Graph
{
public:
    BusGraph();
};

} // namespace photon

#endif // PHOTON_BUSGRAPH_H

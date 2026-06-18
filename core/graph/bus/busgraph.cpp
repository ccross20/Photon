#include "busgraph.h"

namespace photon {

const QByteArray BusGraph::BusGraphId = "bus";

BusGraph::BusGraph()
{
    setName("Bus");
    setGraphTypeId(BusGraphId);
}

} // namespace photon

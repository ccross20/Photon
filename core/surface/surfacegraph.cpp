#include "surfacegraph.h"

namespace photon {

SurfaceGraph::SurfaceGraph(Surface *t_surface) {
    m_surface = t_surface;
    setName("Surface");
}

} // namespace photon

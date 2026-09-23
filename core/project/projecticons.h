#ifndef PHOTON_PROJECTICONS_H
#define PHOTON_PROJECTICONS_H

#include <QIcon>
#include "photon-global.h"

namespace photon {

// Placeholder icons for the Project panel's tree rows: a distinct icon per
// fixture type and per scene helper type, and a swatch/strip representative
// of the actual content for Colors/Gradients/Palettes. Generated procedurally
// since there's no icon set in the app yet - meant to be swapped for real
// artwork later.
PHOTONCORE_EXPORT QIcon projectResourceIcon(ProjectResource *resource);

} // namespace photon

#endif // PHOTON_PROJECTICONS_H

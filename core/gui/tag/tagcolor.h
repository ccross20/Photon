#ifndef PHOTON_TAGCOLOR_H
#define PHOTON_TAGCOLOR_H

#include <QColor>
#include "photon-global.h"

namespace photon {

// Deterministic color for a tag chip's background, derived from the tag's
// text - the same tag always renders the same color, everywhere it's shown,
// with nothing to persist or keep in sync. Case-insensitive (matches
// ProjectResource's tag normalisation).
PHOTONCORE_EXPORT QColor tagColor(const QString &tag);

// Black or white, whichever reads legibly against the given background -
// use for a tag chip's text so it stays readable across every hue tagColor()
// can produce.
PHOTONCORE_EXPORT QColor tagTextColor(const QColor &background);

} // namespace photon

#endif // PHOTON_TAGCOLOR_H

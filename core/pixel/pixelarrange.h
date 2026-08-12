#ifndef PHOTON_PIXELARRANGE_H
#define PHOTON_PIXELARRANGE_H

#include <QVector>
#include <QPointF>
#include "photon-global.h"

namespace photon {

// Pure position-generator functions, no Qt widgets - shared by
// PixelSourceLayout (to seed newly-added/newly-grown pixel slots) and the
// interactive "Arrange" editor commands (to batch-rewrite a selection). Each
// returns exactly `count` points. An Arrange command always recenters the
// result onto the selection's current centroid before committing (see
// PixelLayoutEditorSidePanel::commitArrangedPoints()), so only linear()'s
// absolute placement matters on its own - it's what seeds a freshly added
// source's pixels, unrecentered, and intentionally keeps matching the
// original hardcoded formula this module replaced for that reason.
namespace PixelArrange {

// Evenly spaced along a line of the given length, centered at `center`
// (0..1 fraction of length) and rotated by `angle` degrees. count==1 places
// a single point at the center.
PHOTONCORE_EXPORT QVector<QPointF> linear(int count, double length = .9, double center = .5, double angle = 0);

// Row-major grid: rows*cols should be >= count (excess cells are simply
// unused - the caller decides rows/cols for a given count). Centered at the
// origin, spanning `width` x `height`.
PHOTONCORE_EXPORT QVector<QPointF> grid(int count, int rows, int cols, double width = .9, double height = .9);

// Evenly spaced around a full circle of the given radius - no coincident
// start/end point (count==4 gives a square, not a square plus a repeat).
PHOTONCORE_EXPORT QVector<QPointF> radial(int count, double radius = .45);

// Like radial(), but only sweeps sweepAngle degrees starting at startAngle
// (0 = +X axis, clockwise), with both endpoints included - count==1 places a
// single point at startAngle.
PHOTONCORE_EXPORT QVector<QPointF> arc(int count, double radius = .45, double startAngle = 0, double sweepAngle = 180);

// Row-major grid with every other row offset by half a cell, and rows packed
// at hex-lattice spacing (spacing * sqrt(3)/2 apart vertically) so equally
// spaced neighbors - including diagonal ones - end up the same distance
// apart, the way physically hex-packed LEDs/lenses actually sit.
PHOTONCORE_EXPORT QVector<QPointF> honeycomb(int count, int rows, int cols, double spacing = .2);

// Concentric hexagonal rings growing out from a single center point (ring 0
// = 1 point, ring k>=1 = 6k points) - the compound-eye "bee-eye" cluster
// layout. Fills rings outward until count points are placed; the outermost
// ring may be partial.
PHOTONCORE_EXPORT QVector<QPointF> beeEye(int count, double spacing = .2);

} // namespace PixelArrange

} // namespace photon

#endif // PHOTON_PIXELARRANGE_H

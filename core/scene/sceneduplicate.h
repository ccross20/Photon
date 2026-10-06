#ifndef PHOTON_SCENEDUPLICATE_H
#define PHOTON_SCENEDUPLICATE_H

#include <QVector>
#include "photon-global.h"

namespace photon {

class SceneObject;

// The lowest DMX offset past every currently-patched fixture's channel range
// in the given universe. Not true gap-filling (a hole left by a deleted
// fixture won't be reused) - just append-after-the-highest, which is what
// patching a new or duplicated fixture almost always wants, and cheap to
// compute from the project's flat fixture list rather than walking the scene.
PHOTONCORE_EXPORT int nextAvailableDMXOffset(int universe);

// Clones each object in place, beside the original under the same parent,
// and returns the copies in the same order. Shared by the project panel's
// Duplicate and the visualizer's Cmd-drag.
//
// - An object whose ancestor is also in the list is skipped - it's already
//   copied as part of that ancestor's subtree.
// - Each copy is named with the next free number for its name across the
//   whole scene (SceneObject::nextAvailableName).
// - Cloned fixtures are patched past the highest DMX address in use in their
//   own universe so they don't collide with the fixtures they were copied
//   from. Tracked per universe, so duplicating a mixed-universe selection
//   doesn't scatter later universes' clones onto whatever address an earlier
//   universe happened to reach.
PHOTONCORE_EXPORT QVector<SceneObject*> duplicateSceneObjects(const QVector<SceneObject*> &objects);

} // namespace photon

#endif // PHOTON_SCENEDUPLICATE_H

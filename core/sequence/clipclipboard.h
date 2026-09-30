#ifndef PHOTON_CLIPCLIPBOARD_H
#define PHOTON_CLIPCLIPBOARD_H

#include <QVector>
#include "photon-global.h"

namespace photon {

// Copying sequence clips through the system clipboard, so they can be pasted
// into the same sequence or a different one. Clips keep their spacing in time
// and their offsets between layers.
namespace ClipClipboard {

PHOTONCORE_EXPORT void copy(const QVector<Clip *> &clips);
PHOTONCORE_EXPORT bool hasClips();

// Adds the clipboard's clips to the sequence: the earliest one starts at
// `time`, and the topmost copied layer lands on `targetLayer`. A clip whose
// offset would put it off the end of the layers, or on a non-clip layer, goes
// on `targetLayer` instead. Returns the new clips.
PHOTONCORE_EXPORT QVector<Clip *> paste(Sequence *sequence, ClipLayer *targetLayer, double time);

} // namespace ClipClipboard

} // namespace photon

#endif // PHOTON_CLIPCLIPBOARD_H

#ifndef PHOTON_TAGMIME_H
#define PHOTON_TAGMIME_H

#include "photon-global.h"

class QMimeData;

namespace photon {

// Mime format for dragging one or more tag names - out of a TagChip, or a
// tree row's painted chip - onto whatever accepts a tag drop (another
// resource's row in the Project panel tree, or any TagEditorWidget).
PHOTONCORE_EXPORT extern const QByteArray TagMimeType;

// Encode/decode helpers mirroring encodeSceneObjectMime/decodeSceneObjectMime
// (core/scene/scenemodel.h) - same QDataStream shape and PID-guard
// convention, kept consistent with the rest of the app's drag/drop even
// though tag names (unlike scene-object pointers) would survive a
// cross-process drop just fine.
PHOTONCORE_EXPORT QMimeData *encodeTagMime(const QStringList &tags);
PHOTONCORE_EXPORT QStringList decodeTagMime(const QMimeData *mimeData);

} // namespace photon

#endif // PHOTON_TAGMIME_H

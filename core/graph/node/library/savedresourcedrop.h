#ifndef PHOTON_SAVEDRESOURCEDROP_H
#define PHOTON_SAVEDRESOURCEDROP_H

#include "keira-global.h"
#include "photon-global.h"

class QMimeData;

namespace photon {

// Decodes a Color/Gradient/Color Palette row dragged from the project panel
// (ProjectResource::ProjectResourceMime) into the matching Saved*Node, or a
// tag chip dragged from the project panel (photon::TagMimeType) into a
// SelectFixturesNode with that tag preset, or a Falloff scene object into a
// Spatial Falloff node pointed at it, so any of them can be created by
// dropping straight into a node graph. Wire this onto every keira::Scene via
// Scene::setExternalDropInterpreter.
PHOTONCORE_EXPORT QVector<keira::ExternalDropNodeSpec> projectResourceDropInterpreter(const QMimeData *mimeData);

} // namespace photon

#endif // PHOTON_SAVEDRESOURCEDROP_H

#ifndef PHOTON_PROPERTYCONTROLLER_H
#define PHOTON_PROPERTYCONTROLLER_H

#include <QObject>
#include <memory>
#include "photon-global.h"

namespace keira { class Node; }

namespace photon {

class PropertySubject;
class Surface;
class SurfaceGizmo;
class ProjectResource;
class ChannelEffect;
class Clip;

// The single place selection arrives from, for every kind of editable thing.
//
// Selection sources - the graph scene, the surface designer, the project panel,
// the visualizer - push what they selected here, and the Properties panel
// listens. That inversion is the point of the consolidation: sources no longer
// own a property sidebar each, and none of them knows the panel exists.
//
// A process-wide singleton rather than something hung off Project, because a
// graph node's selection outlives any single project open/close cycle and the
// sources are spread across core and plugins.
class PHOTONCORE_EXPORT PropertyController : public QObject
{
    Q_OBJECT
public:
    static PropertyController *instance();

    // Takes ownership. Passing nullptr clears the selection.
    void setSubject(PropertySubject *subject);
    PropertySubject *subject() const;

    // Convenience wrappers so call sites don't need the factory.
    void selectNode(keira::Node *node);
    void selectGizmo(Surface *surface, SurfaceGizmo *gizmo);
    void selectResource(ProjectResource *resource);
    void selectChannelEffect(ChannelEffect *effect);
    void selectClip(Clip *clip);
    void clear();

signals:
    // The subject pointer stays owned by the controller; listeners must not
    // store it beyond the next change.
    void subjectChanged(photon::PropertySubject *subject);

private:
    PropertyController();
    // Out of line: unique_ptr's deleter needs PropertySubject complete, and
    // this header only forward-declares it.
    ~PropertyController() override;

    std::unique_ptr<PropertySubject> m_subject;
};

} // namespace photon

#endif // PHOTON_PROPERTYCONTROLLER_H

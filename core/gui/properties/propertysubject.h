#ifndef PHOTON_PROPERTYSUBJECT_H
#define PHOTON_PROPERTYSUBJECT_H

#include <QString>
#include <QPointer>
#include "photon-global.h"
#include "propertyaddress.h"

class QWidget;

namespace keira { class Graph; class Node; }

namespace photon {

class Surface;
class SurfaceGizmo;
class ProjectResource;
class Channel;
class ChannelEffect;

// One editable thing, as the Properties panel sees it.
//
// A subject deliberately holds no pointer to its target. It stores the target's
// address and re-resolves on demand, which is what lets a pinned tab from a
// previous session behave exactly like a live selection, and means nothing
// dangles when the target is deleted - isValid() simply starts returning false.
class PHOTONCORE_EXPORT PropertySubject
{
public:
    virtual ~PropertySubject() = default;

    virtual PropertyAddress address() const = 0;
    // Shown on the tab and as the last breadcrumb. Re-read live, so a rename
    // is reflected without rebuilding the subject.
    virtual QString title() const = 0;
    // False once the target can no longer be found - deleted, or its owning
    // project closed. The panel greys out a pinned tab rather than dropping it.
    virtual bool isValid() const = 0;
    // Builds the page. Caller takes ownership. Returns nullptr when invalid.
    virtual QWidget *createEditor() = 0;
};

// A node inside a graph. Held as graph + node id because keira::Node is not a
// QObject, so there is no QPointer to guard it with.
class PHOTONCORE_EXPORT NodePropertySubject : public PropertySubject
{
public:
    NodePropertySubject(keira::Graph *graph, const QByteArray &nodeId);

    PropertyAddress address() const override;
    QString title() const override;
    bool isValid() const override;
    QWidget *createEditor() override;

    keira::Node *node() const;

private:
    QPointer<keira::Graph> m_graph;
    QByteArray m_nodeId;
};

// A gizmo inside a surface.
class PHOTONCORE_EXPORT GizmoPropertySubject : public PropertySubject
{
public:
    GizmoPropertySubject(Surface *surface, const QByteArray &gizmoId);

    PropertyAddress address() const override;
    QString title() const override;
    bool isValid() const override;
    QWidget *createEditor() override;

    SurfaceGizmo *gizmo() const;

private:
    QPointer<Surface> m_surface;
    QByteArray m_gizmoId;
};

// Any ProjectResource - scene object, routine, sequence, surface, pixel layout,
// fixture group. Delegates the page to createResourceEditor().
class PHOTONCORE_EXPORT ResourcePropertySubject : public PropertySubject
{
public:
    explicit ResourcePropertySubject(ProjectResource *resource);

    PropertyAddress address() const override;
    QString title() const override;
    bool isValid() const override;
    QWidget *createEditor() override;

    ProjectResource *resource() const;

private:
    QByteArray m_resourceId;
    QByteArray m_typeId;
    // Guards against the resource being destroyed while we hold it; resourceObject()
    // is the QObject every ProjectResource is mixed into.
    QPointer<QObject> m_guard;
};

// A channel effect's plain parameter fields (createPropertyEditor()) - not the
// interactive curve/gizmo editor (createEditor()), which stays inline in the
// sequence timeline rather than moving into the Properties panel.
//
// ChannelEffect isn't a QObject, so it's held the same way NodePropertySubject
// holds a keira::Node: a QPointer to its stable, QObject owner (the Channel)
// plus the effect's own uniqueId, re-resolved by lookup on every access.
class PHOTONCORE_EXPORT ChannelEffectPropertySubject : public PropertySubject
{
public:
    ChannelEffectPropertySubject(Channel *channel, const QByteArray &effectId);

    PropertyAddress address() const override;
    QString title() const override;
    bool isValid() const override;
    QWidget *createEditor() override;

    ChannelEffect *effect() const;

private:
    QPointer<Channel> m_channel;
    QByteArray m_effectId;
};

// Turns a stored address back into a live subject. Used when restoring pinned
// tabs from the project file, and by breadcrumb navigation.
namespace PropertySubjectFactory {

// Null when the address cannot be resolved in the current project.
PHOTONCORE_EXPORT PropertySubject *resolve(const PropertyAddress &address);

// Convenience constructors that build the full ancestry into the address.
PHOTONCORE_EXPORT PropertySubject *forNode(keira::Node *node);
PHOTONCORE_EXPORT PropertySubject *forGizmo(Surface *surface, SurfaceGizmo *gizmo);
PHOTONCORE_EXPORT PropertySubject *forResource(ProjectResource *resource);
PHOTONCORE_EXPORT PropertySubject *forChannelEffect(ChannelEffect *effect);

} // namespace PropertySubjectFactory

} // namespace photon

#endif // PHOTON_PROPERTYSUBJECT_H

#ifndef PHOTON_PROJECTRESOURCE_H
#define PHOTON_PROJECTRESOURCE_H

#include <QObject>
#include <QStringList>
#include "photon-global.h"

class QWidget;

namespace photon {

// Change notification for a ProjectResource. Lives in its own QObject because
// ProjectResource itself cannot be one - see the note on that class - and the
// project model needs a single signal it can connect to without knowing the
// concrete resource type.
class PHOTONCORE_EXPORT ProjectResourceNotifier : public QObject
{
    Q_OBJECT
public:
    explicit ProjectResourceNotifier(ProjectResource *resource, QObject *parent = nullptr);

    ProjectResource *resource() const;

signals:
    // Name, tags, or anything else the project panel displays has changed.
    void resourceChanged(photon::ProjectResource *);

private:
    ProjectResource *m_resource;
};

// The common contract for anything that appears in the project as an asset:
// scene objects, routines, sequences, surfaces, pixel layouts, states, fixture
// groups - and, later, palettes.
//
// Deliberately NOT a QObject. Routine already derives from keira::Graph, which
// is one, and a class cannot inherit QObject twice. Concrete types therefore
// mix this in alongside whatever QObject base they already have, and expose
// change notifications through resourceNotifier().
//
// The accessors are `resource`-prefixed on purpose: keira::Graph, SceneObject,
// Surface, PixelLayout and State each already declare their own uniqueId() and
// name() independently, and unprefixed names here would be ambiguous at every
// call site. Concrete types forward in one line.
class PHOTONCORE_EXPORT ProjectResource
{
public:
    ProjectResource();
    virtual ~ProjectResource();

    virtual QByteArray resourceId() const = 0;
    // Stable type tag used to group resources and to pick an icon - "fixture",
    // "routine", "sequence", "surface", "pixel-layout", "state", "group", ...
    virtual QByteArray resourceTypeId() const = 0;
    virtual QString resourceName() const = 0;
    virtual void setResourceName(const QString &) = 0;
    // The concrete object, for connect() and for lifetime tracking. Every
    // resource is a QObject of some kind, just not through this base.
    virtual QObject *resourceObject() = 0;

    // Editor shown in the Properties panel when this resource is selected.
    // The caller takes ownership. The default builds a name + tags editor,
    // which is a reasonable floor for any resource.
    virtual QWidget *createResourceEditor();

    // Tags are stored here so every resource type gets them for free. Values
    // are normalised to lower case and kept sorted, so a tag reads the same
    // whichever type it was applied to.
    const QStringList &resourceTags() const;
    void setResourceTags(const QStringList &);
    void addResourceTag(const QString &);
    void removeResourceTag(const QString &);
    bool hasResourceTag(const QString &) const;

    ProjectResourceNotifier *resourceNotifier() const;
    // Called whenever shared metadata changes. Concrete types also call it from
    // their own change paths so the project panel refreshes.
    //
    // Virtual so a type that already has a metadata signal of its own can route
    // through it instead of having two parallel notification paths - see
    // SceneObject, which redirects this into metadataChanged() and bridges back
    // via ProjectResource::notifyResourceChanged() on the far side.
    virtual void notifyResourceChanged();

    // Shared read/write of the "tags" array. Concrete types call these from
    // their own readFromJson/writeToJson.
    void readResourceJson(const QJsonObject &);
    void writeResourceJson(QJsonObject &) const;

private:
    QStringList m_tags;
    ProjectResourceNotifier *m_notifier;
};

} // namespace photon

#endif // PHOTON_PROJECTRESOURCE_H

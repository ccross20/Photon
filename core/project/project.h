#ifndef PHOTON_PROJECT_H
#define PHOTON_PROJECT_H

#include "photon-global.h"

class QWidget;

namespace photon {

class SceneManager;

class PHOTONCORE_EXPORT Project : public QObject
{
    Q_OBJECT
public:
    explicit Project(QObject *parent = nullptr);
    ~Project();

    FixtureCollection *fixtures() const;
    RoutineCollection *routines() const;
    PixelLayoutCollection *pixelLayouts() const;
    BusGraph *bus() const;
    SceneObject *sceneRoot() const;
    SceneManager *scene() const;
    FixtureGroupCollection *groups() const;
    SurfaceCollection *surfaces() const;

    // Every tag currently in use anywhere in the project - every scene object,
    // fixture group, routine, surface, pixel layout and sequence - deduped and
    // sorted. Derived live rather than kept in a separate vocabulary, so it can
    // never drift out of sync with what's actually on resources. The source
    // for every tag picker's autocomplete pool.
    QStringList allTags() const;

    // The project-wide selection, over any kind of resource — scene objects,
    // routines, sequences, surfaces, ... The scene-object accessors below are
    // a filtered view of this, kept for the many consumers that only deal in
    // SceneObjects (visualizer viewport, DMX patch grid, falloff editor).
    ProjectResource *selectedResource() const;
    void setSelectedResource(ProjectResource *);
    QList<ProjectResource*> selectedResources() const;
    void setSelectedResources(const QList<ProjectResource*> &);

    // The primary (most-recently-selected) object — the last entry of
    // selectedSceneObjects(), or nullptr if nothing is selected.
    SceneObject *selectedSceneObject() const;
    // Convenience for single-selection: replaces the whole selection with
    // just this object (or clears it, for nullptr).
    void setSelectedSceneObject(SceneObject *obj);

    QList<SceneObject*> selectedSceneObjects() const;
    void setSelectedSceneObjects(const QList<SceneObject*> &objs);

    // The widget shown in the Properties/Attributes panel. Any editor (scene-object
    // selection, a selected channel effect, ...) can set it; the Project owns it and
    // deletes the previous one. Set nullptr to clear. This lets one shared panel
    // serve multiple selection sources.
    QWidget *propertiesWidget() const;
    void setPropertiesWidget(QWidget *widget);

    void save(const QString &path = QString{}) const;
    void load(const QString &path = QString{});
    void restore(Project &);
    void readFromJson(const QJsonObject &json);
    void writeToJson(QJsonObject &json) const;

signals:
    // The full selection, of any resource type. The Properties panel listens
    // here so it can edit anything, not just scene objects.
    void selectedResourceChanged(photon::ProjectResource *);
    void selectedResourcesChanged(const QList<photon::ProjectResource*> &);

    // Fired whenever the selection changes, alongside selectedSceneObjectsChanged
    // — carries just the primary object, for consumers that only care about a
    // single selected object (e.g. the visualizer). These see only the
    // SceneObject part of the selection; selecting a routine reports an empty
    // scene selection rather than leaving a stale one in place.
    void selectedSceneObjectChanged(photon::SceneObject *);
    void selectedSceneObjectsChanged(const QList<photon::SceneObject*> &objs);
    // The Properties-panel widget changed. The panel displays the new widget; the
    // Project owns and deletes the old one.
    void propertiesWidgetChanged(QWidget *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROJECT_H

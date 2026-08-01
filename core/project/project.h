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
    CanvasCollection *canvases() const;
    PixelLayoutCollection *pixelLayouts() const;
    StateCollection *states() const;
    BusGraph *bus() const;
    SceneObject *sceneRoot() const;
    SceneManager *scene() const;
    TagCollection *tags() const;
    FixtureGroupCollection *groups() const;
    SurfaceCollection *surfaces() const;

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
    // Fired whenever the selection changes, alongside selectedSceneObjectsChanged
    // — carries just the primary object, for consumers that only care about a
    // single selected object (e.g. the Properties panel, the visualizer).
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

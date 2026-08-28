#ifndef PHOTON_SCENEZONE_H
#define PHOTON_SCENEZONE_H

#include <QWidget>
#include <QVector3D>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class SceneZone;

class SceneZoneEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneZoneEditorWidget(SceneZone *, QWidget *parent = nullptr);
    ~SceneZoneEditorWidget();

private slots:
    void setName(const QString &name);
    void setSize(const QVector3D &);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);
    void refreshTransform();
    void refreshSize();

private:
    class Impl;
    Impl *m_impl;
};

// A named 3D region (an oriented box) used to select fixtures spatially. The box is
// centered at the object origin with full extents = size(); orient/position it with
// the transform gizmo. A fixture is "in" the zone when its world position is inside.
// Color and visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT SceneZone : public SceneHelperObject
{
    Q_OBJECT
public:
    SceneZone();
    ~SceneZone();

    void setSize(const QVector3D &);

    QVector3D size() const;

    // True if the given WORLD point lies inside the (possibly rotated) box.
    bool containsPoint(const QVector3D &worldPoint) const;

    // Finds a zone by name anywhere in the project's scene (null if none).
    static SceneZone *findByName(Project *project, const QString &name);
    // All zone names in the project's scene (for selection dropdowns).
    static QStringList zoneNames(Project *project);

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEZONE_H

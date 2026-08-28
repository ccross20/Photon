#ifndef PHOTON_SCENEHELPEROBJECT_H
#define PHOTON_SCENEHELPEROBJECT_H

#include <QColor>
#include "photon-global.h"
#include "scene/sceneobject.h"

class QFormLayout;

namespace photon {

// Common base for the non-fixture annotation/marker objects a rig is laid
// out with (Zone, Arrow, Direction, Axis, Boundary Rectangle/Oval, Point
// Marker): a user-editable color and a visibility mode. Everything else
// (dimensions - a box size, a width/height, a length, an icon shape...) is
// too shape-specific to force into one common field, so each concrete type
// still declares its own, exactly as SceneZone/SceneSurface already did
// before this class existed.
class PHOTONCORE_EXPORT SceneHelperObject : public SceneObject
{
    Q_OBJECT
public:
    enum VisibilityMode { AlwaysVisible, SelectedOnly };

    QColor color() const;
    void setColor(const QColor &);

    VisibilityMode visibilityMode() const;
    void setVisibilityMode(VisibilityMode);

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

protected:
    explicit SceneHelperObject(const QByteArray &typeId, SceneObject *parent = nullptr);

private:
    class Impl;
    Impl *m_impl;
};

// Appends "Color" and "Visible" rows to an existing form layout, wired to the
// given helper object (color swatch button -> QColorDialog, visibility ->
// combo box), including a metadataChanged hookup to keep the swatch live.
// Shared by every SceneHelperObject-derived editor instead of re-implementing
// the same two widgets in each one.
PHOTONCORE_EXPORT void addHelperPropertyRows(QFormLayout *form, SceneHelperObject *object, QWidget *editorParent);

} // namespace photon

#endif // PHOTON_SCENEHELPEROBJECT_H

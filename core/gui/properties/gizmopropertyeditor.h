#ifndef PHOTON_GIZMOPROPERTYEDITOR_H
#define PHOTON_GIZMOPROPERTYEDITOR_H

#include <QWidget>
#include <QPointer>
#include <QHash>
#include <functional>
#include "photon-global.h"
#include "surface/gizmoproperty.h"

namespace photon {

class SurfaceGizmo;
class PropertyForm;

// Property page for a SurfaceGizmo.
//
// Replaces the old QML inspector (core/qml/surface/Inspector.qml). Built on
// PropertyForm and the shared PropertyWidgets factory so a gizmo's number field
// is the same NumberScrubField a node parameter gets, rather than a QML
// TextField that merely looked similar.
//
// Properties are grouped by their "category" metadata, in the same order the
// QML inspector used: Identity, Layout, Style, then everything else.
class PHOTONCORE_EXPORT GizmoPropertyEditor : public QWidget
{
    Q_OBJECT
public:
    explicit GizmoPropertyEditor(SurfaceGizmo *gizmo, QWidget *parent = nullptr);

private slots:
    // Pushes an externally-driven change (dragging the gizmo on the canvas
    // moves x/y) into the matching field, without rebuilding the page - a
    // rebuild would destroy the widget the user is mid-edit in.
    void propertyChangedExternally(const QByteArray &id);

private:
    void rebuild();
    void addProperty(PropertyForm *form, GizmoProperty *prop);

    QPointer<SurfaceGizmo> m_gizmo;
    PropertyForm *m_form = nullptr;

    // id -> "write this value into the field". Populated as the page is built.
    QHash<QByteArray, std::function<void(const QVariant &)>> m_setters;
    // Guards the echo: our own write emits propertyChanged, which would
    // otherwise bounce straight back into the field being edited.
    bool m_applying = false;
};

} // namespace photon

#endif // PHOTON_GIZMOPROPERTYEDITOR_H

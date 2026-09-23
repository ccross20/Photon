#ifndef PHOTON_PROPERTIESPANEL_H
#define PHOTON_PROPERTIESPANEL_H

#include "photon-global.h"
#include "gui/panel.h"
#include "gui/properties/propertyaddress.h"

namespace photon {

class PropertySubject;
class ProjectResource;

// The one place properties are edited, for every kind of object.
//
// Cinema-4D-style "Attributes" panel, extended into the single property surface
// for the whole app: graph nodes, surface gizmos and project resources all show
// here, built from the same widget factory so they look and behave alike. The
// node editor and surface designer no longer carry sidebars of their own.
//
// Three parts: a breadcrumb address bar showing where the edited object lives,
// a tab strip (one tab that follows the selection plus any number of pinned
// ones), and the page itself.
class PHOTONCORE_EXPORT PropertiesPanel : public Panel
{
    Q_OBJECT
public:
    PropertiesPanel();
    ~PropertiesPanel();

private slots:
    void subjectChanged(photon::PropertySubject *subject);
    void selectedResourceChanged(photon::ProjectResource *resource);
    void tabChanged(int index);
    void tabClosed(int index);
    void togglePin();
    void addressSegmentActivated(const photon::PropertyAddress::Segment &segment, int index);

private:
    // Builds and installs the page for `subject` (or an empty state). Separate
    // from subjectChanged() because switching to a pinned tab shows a subject
    // the controller never emitted.
    void showSubject(photon::PropertySubject *subject);
    void savePinnedTabs();
    void restorePinnedTabs();

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROPERTIESPANEL_H

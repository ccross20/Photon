#ifndef PHOTON_PROPERTIESPANEL_H
#define PHOTON_PROPERTIESPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

// Cinema-4D-style "Attributes" panel: shows the property editor for whatever
// resource is currently selected, via Project::selectedResource. Kept separate
// from any panel that changes the selection so multiple selection sources
// (project panel, visualizer viewport, DMX patch grid) share one editor.
class PHOTONCORE_EXPORT PropertiesPanel : public Panel
{
public:
    PropertiesPanel();
    ~PropertiesPanel();

private slots:
    void selectedResourceChanged(photon::ProjectResource *resource);
    void propertiesWidgetChanged(QWidget *widget);

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROPERTIESPANEL_H

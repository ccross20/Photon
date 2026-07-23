#ifndef PHOTON_PROPERTIESPANEL_H
#define PHOTON_PROPERTIESPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

// Cinema-4D-style "Attributes" panel: shows the property editor for whatever
// scene object is currently selected, via Project::selectedSceneObject. Kept
// separate from RigPanel (or any other panel that changes the selection) so
// multiple selection sources can eventually share it.
class PHOTONCORE_EXPORT PropertiesPanel : public Panel
{
public:
    PropertiesPanel();
    ~PropertiesPanel();

private slots:
    void selectedObjectChanged(photon::SceneObject *obj);

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROPERTIESPANEL_H

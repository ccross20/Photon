#ifndef PHOTON_RIGPANEL_H
#define PHOTON_RIGPANEL_H

#include <QItemSelection>
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class RigPanel : public Panel
{
public:
    RigPanel();
    ~RigPanel();

private slots:
    void addClicked();
    void addFixture();
    void addGroup();
    void addTruss();
    void addLightStrip();
    void addArrow();
    void addWall();
    void addFloor();
    void addZone();
    void removeClicked();
    void duplicateClicked();
    void doubleClicked(const QModelIndex &);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void selectionMoved(QModelIndexList indices);
    void syncSelectionFromProject(photon::SceneObject *obj);

protected:
    void projectDidOpen(photon::Project* project) override;
    void projectWillClose(photon::Project* project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_RIGPANEL_H

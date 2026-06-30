#ifndef PHOTON_FIXTUREGROUPCOLLECTIONPANEL_H
#define PHOTON_FIXTUREGROUPCOLLECTIONPANEL_H

#include <QItemSelection>
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

// Manages named, project-level fixture groups (saved FixtureQuerys): add / remove,
// and double-click to edit a group's tag/type query.
class PHOTONCORE_EXPORT FixtureGroupCollectionPanel : public Panel
{
public:
    FixtureGroupCollectionPanel();
    ~FixtureGroupCollectionPanel();

private slots:
    void addClicked();
    void removeClicked();
    void doubleClicked(const QModelIndex &);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

protected:
    void projectDidOpen(photon::Project* project) override;
    void projectWillClose(photon::Project* project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREGROUPCOLLECTIONPANEL_H

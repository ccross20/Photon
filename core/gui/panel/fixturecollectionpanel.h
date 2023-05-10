#ifndef PHOTON_FIXTURECOLLECTIONPANEL_H
#define PHOTON_FIXTURECOLLECTIONPANEL_H

#include <QItemSelection>
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class FixtureCollectionPanel : public Panel
{
public:
    FixtureCollectionPanel();
    ~FixtureCollectionPanel();

private slots:
    void addClicked();
    void addFixture();
    void addGroup();
    void addTruss();
    void removeClicked();
    void duplicateClicked();
    void doubleClicked(const QModelIndex &);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void selectionMoved(QModelIndexList indices);

protected:
    void projectDidOpen(photon::Project* project) override;
    void projectWillClose(photon::Project* project) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECOLLECTIONPANEL_H

#ifndef SURFACECOLLECTIONPANEL_H
#define SURFACECOLLECTIONPANEL_H

#include <QItemSelection>
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class SurfaceCollectionPanel : public Panel
{
public:
    SurfaceCollectionPanel();
    ~SurfaceCollectionPanel();

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

#endif // SURFACECOLLECTIONPANEL_H





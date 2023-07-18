#ifndef PHOTON_PIXELLAYOUTCOLLECTIONPANEL_H
#define PHOTON_PIXELLAYOUTCOLLECTIONPANEL_H

#include <QItemSelection>
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {


class PixelLayoutCollectionPanel : public Panel
{
public:
    PixelLayoutCollectionPanel();
    ~PixelLayoutCollectionPanel();

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

#endif // PHOTON_PIXELLAYOUTCOLLECTIONPANEL_H

#ifndef PHOTON_TAGCOLLECTIONPANEL_H
#define PHOTON_TAGCOLLECTIONPANEL_H

#include <QItemSelection>
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class PHOTONCORE_EXPORT TagCollectionPanel : public Panel
{
public:
    TagCollectionPanel();
    ~TagCollectionPanel();

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

#endif // PHOTON_TAGCOLLECTIONPANEL_H

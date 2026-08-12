#ifndef PHOTON_PROJECTPANEL_H
#define PHOTON_PROJECTPANEL_H

#include <QModelIndexList>
#include "photon-global.h"
#include "gui/panel.h"

class QEvent;
class QItemSelection;
class QMenu;
class QModelIndex;

namespace photon {

// One panel for every asset in the project - the rig hierarchy plus fixture
// groups, routines, sequences, surfaces and pixel layouts - with a search/tag
// filter over the lot. Replaces the per-collection panels; selection feeds the
// shared Properties panel through Project::setSelectedResources().
class PHOTONCORE_EXPORT ProjectPanel : public Panel
{
    Q_OBJECT
public:
    ProjectPanel();
    ~ProjectPanel();

protected:
    void projectDidOpen(photon::Project *project) override;
    void projectWillClose(photon::Project *project) override;
    // Detects a press-and-drag over a tag chip in the tree's Tags column and
    // turns it into a QDrag - independent of the model's own row-drag
    // machinery, which only fires for whole-row drags.
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void syncSelectionFromProject(const QList<photon::ProjectResource*> &resources);
    void selectionMoved(QModelIndexList indices);
    void doubleClicked(const QModelIndex &index);
    void addClicked();
    void removeClicked();
    void duplicateClicked();
    void filterChanged();
    void contextMenuRequested(const QPoint &pos);

private:
    // Adds the "New <thing>" action(s) for one category directly to menu - no
    // wrapping submenu. Used both as the whole content of a category-specific
    // context menu, and as the content of each submenu in the full Add menu.
    // preferredParent overrides where new scene objects land (the row that was
    // right-clicked); null falls back to the current selection, then the scene
    // root, evaluated when the action actually runs.
    void populateAddActions(QMenu &menu, const QByteArray &contentType, photon::SceneObject *preferredParent = nullptr);
    // Every category, each in its own submenu - what the toolbar Add button
    // shows, since it isn't scoped to anything the user clicked.
    void populateFullAddMenu(QMenu &menu, photon::SceneObject *preferredParent = nullptr);

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROJECTPANEL_H

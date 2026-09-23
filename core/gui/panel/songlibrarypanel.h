#ifndef SONGLIBRARYPANEL_H
#define SONGLIBRARYPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

class QTreeWidgetItem;
class QPoint;

namespace photon {

// Manages the app-level SongLibrary (photonApp->songLibrary()): browse songs
// (as a tree, expanded into child sequence rows only when a song has more
// than one) and, via right-click, create/remove sequences and pick a song's
// default. Independent of any open Project - the library is opened from
// ApplicationSettings::songDataLibraryPath(), so this panel is usable (or
// prompts to configure a path) regardless of project state.
class SongLibraryPanel : public Panel
{
    Q_OBJECT
public:
    SongLibraryPanel();
    ~SongLibraryPanel();

private slots:
    void importVdjClicked();
    void cancelImportClicked();
    void importTick();

    void setDefaultClicked();
    void newSequenceClicked();
    void removeClicked();
    void itemDoubleClicked(QTreeWidgetItem *);
    void showContextMenu(const QPoint &);

private:
    void refreshTree();
    void openLibraryPrompt();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SONGLIBRARYPANEL_H

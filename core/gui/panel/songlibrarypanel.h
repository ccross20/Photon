#ifndef SONGLIBRARYPANEL_H
#define SONGLIBRARYPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

class QListWidgetItem;

namespace photon {

// Manages the app-level SongLibrary (photonApp->songLibrary()): browse/add/
// remove catalogued songs, and for the selected song, link/create sequences
// and choose which one is the default. Independent of any open Project - the
// library is opened from ApplicationSettings::songDataLibraryPath(), so this
// panel is usable (or prompts to configure a path) regardless of project state.
class SongLibraryPanel : public Panel
{
    Q_OBJECT
public:
    SongLibraryPanel();
    ~SongLibraryPanel();

private slots:
    void addLocalClicked();
    void importVdjClicked();
    void cancelImportClicked();
    void importTick();
    void removeSongClicked();
    void songSelectionChanged();

    void setDefaultClicked();
    void addExistingClicked();
    void newSequenceClicked();
    void removeSequenceClicked();
    void sequenceDoubleClicked(QListWidgetItem *);

private:
    void refreshSelectedSong();
    void openLibraryPrompt();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SONGLIBRARYPANEL_H

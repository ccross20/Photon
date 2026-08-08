#ifndef SONGLIBRARYPANEL_P_H
#define SONGLIBRARYPANEL_P_H

#include <QListView>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "songlibrarypanel.h"

namespace photon {

class SongLibrary;
class Sequence;
class VirtualDJCaptureProcess;

// Wraps SongLibrary::songs() by row index (not by pointer - see the pointer-
// lifetime caveat on SongLibrary's find/add methods). SongLibrary's mutation
// signals fire AFTER the underlying QVector has already changed (unlike the
// *Collection classes' will/was-added pairs), so a full reset on any change
// is the simplest correct response - hundreds of rows makes that cheap.
class SongLibraryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SongLibraryModel(SongLibrary *library);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // The song id at a given row, or -1 if out of range.
    qint64 idAt(int row) const;

private slots:
    void reset();

private:
    SongLibrary *m_library;
};

class SongLibraryPanel::Impl
{
public:
    SongLibraryModel *model = nullptr;

    QListView *songList = nullptr;
    QPushButton *addLocalButton = nullptr;
    QPushButton *importVdjButton = nullptr;
    QPushButton *cancelImportButton = nullptr;
    QProgressBar *importProgress = nullptr;
    QLabel *importRemainingLabel = nullptr;
    QPushButton *removeSongButton = nullptr;

    // The active VirtualDJ capture, if any. captureSequence is a scratch
    // Sequence never added to any collection - just the carrier
    // VirtualDJCaptureProcess needs (it writes into sequence()->songData()).
    // captureSourcePath is snapshotted at start, since the process itself
    // leaves SongData::sourcePath blank (the streaming-track case) and its
    // own metadata snapshot isn't otherwise reachable from outside.
    VirtualDJCaptureProcess *captureProcess = nullptr;
    Sequence *captureSequence = nullptr;
    QString captureSourcePath;

    QLabel *titleLabel = nullptr;
    QLabel *artistLabel = nullptr;
    QLabel *durationLabel = nullptr;
    QLabel *sourceLabel = nullptr;

    QListWidget *sequenceList = nullptr;
    QPushButton *setDefaultButton = nullptr;
    QPushButton *addExistingButton = nullptr;
    QPushButton *newSequenceButton = nullptr;
    QPushButton *removeSequenceButton = nullptr;

    // -1 = no song selected. Kept as an id (not a pointer) since
    // SongLibraryEntry* can be invalidated by any later add/remove.
    qint64 selectedSongId = -1;
};

} // namespace photon

#endif // SONGLIBRARYPANEL_P_H

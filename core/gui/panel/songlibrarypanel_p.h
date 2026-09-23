#ifndef SONGLIBRARYPANEL_P_H
#define SONGLIBRARYPANEL_P_H

#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "songlibrarypanel.h"

namespace photon {

class SongLibrary;
class Sequence;
class VirtualDJCaptureProcess;

class SongLibraryPanel::Impl
{
public:
    QTreeWidget *songTree = nullptr;

    QPushButton *importVdjButton = nullptr;
    QPushButton *cancelImportButton = nullptr;
    QProgressBar *importProgress = nullptr;
    QLabel *importRemainingLabel = nullptr;

    // The active VirtualDJ capture, if any. captureSequence is a scratch
    // Sequence never added to any collection - just the carrier
    // VirtualDJCaptureProcess needs (it writes into sequence()->songData()).
    // captureSourcePath is snapshotted at start, since the process itself
    // leaves SongData::sourcePath blank (the streaming-track case) and its
    // own metadata snapshot isn't otherwise reachable from outside.
    VirtualDJCaptureProcess *captureProcess = nullptr;
    Sequence *captureSequence = nullptr;
    QString captureSourcePath;
};

} // namespace photon

#endif // SONGLIBRARYPANEL_P_H

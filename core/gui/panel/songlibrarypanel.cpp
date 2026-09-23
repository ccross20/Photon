#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QDir>
#include <QMenu>
#include <QRegularExpression>
#include "songlibrarypanel_p.h"
#include "library/songlibrary.h"
#include "photoncore.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"
#include "virtualdj/virtualdjconnector.h"
#include "gui/dialog/settingsdialog.h"
#include "audio/virtualdjcaptureprocess.h"
#include "audio/songdata.h"
#include "timekeeper.h"

namespace photon {

namespace {

// Data roles on QTreeWidgetItem: every item carries an id (song id for a
// song row, sequence id for a sequence row) and a kind so slots can tell
// which without re-deriving it from shape. Sequence rows also carry their
// parent song's id, since SongLibrary's sequence calls are keyed by
// (songId, sequenceId) pairs, not sequence id alone.
constexpr int IdRole = Qt::UserRole;
constexpr int KindRole = Qt::UserRole + 1;
constexpr int SongIdRole = Qt::UserRole + 2;

enum ItemKind { SongItem, SequenceItem };

QString songLabel(const SongLibraryEntry &t_song)
{
    return t_song.artist.isEmpty() ? t_song.title : t_song.title + "  -  " + t_song.artist;
}

QString sequenceLabel(const SongLibrarySequenceEntry &t_sequence)
{
    return t_sequence.isDefault ? t_sequence.name + "  (Default)" : t_sequence.name;
}

QString sanitizedFileStem(const QString &t_name)
{
    QString result = t_name;
    result.replace(QRegularExpression("[^A-Za-z0-9 _-]"), "_");
    return result.isEmpty() ? "Untitled" : result;
}

QString formatRemaining(double t_seconds)
{
    const int total = qMax(0, int(t_seconds));
    return QString("%1:%2 remaining").arg(total / 60).arg(total % 60, 2, 10, QChar('0'));
}

} // namespace

SongLibraryPanel::SongLibraryPanel() : Panel("photon.song-library"), m_impl(new Impl)
{
    setName("Song Library");

    SongLibrary *library = photonApp->songLibrary();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_impl->songTree = new QTreeWidget;
    m_impl->songTree->setHeaderHidden(true);
    m_impl->songTree->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_impl->songTree->setContextMenuPolicy(Qt::CustomContextMenu);
    mainLayout->addWidget(m_impl->songTree);

    m_impl->importVdjButton = new QPushButton("Import VDJ Song");
    m_impl->cancelImportButton = new QPushButton("Cancel");
    m_impl->cancelImportButton->setVisible(false);

    QHBoxLayout *importButtons = new QHBoxLayout;
    importButtons->addWidget(m_impl->importVdjButton);
    importButtons->addWidget(m_impl->cancelImportButton);
    importButtons->addStretch();
    mainLayout->addLayout(importButtons);

    m_impl->importProgress = new QProgressBar;
    m_impl->importProgress->setVisible(false);
    m_impl->importProgress->setTextVisible(false);
    mainLayout->addWidget(m_impl->importProgress);

    m_impl->importRemainingLabel = new QLabel;
    m_impl->importRemainingLabel->setVisible(false);
    mainLayout->addWidget(m_impl->importRemainingLabel);

    setPanelLayout(mainLayout);

    connect(m_impl->importVdjButton, &QPushButton::clicked, this, &SongLibraryPanel::importVdjClicked);
    connect(m_impl->cancelImportButton, &QPushButton::clicked, this, &SongLibraryPanel::cancelImportClicked);
    connect(photonApp->timekeeper(), &Timekeeper::tick, this, &SongLibraryPanel::importTick);
    connect(m_impl->songTree, &QTreeWidget::itemDoubleClicked, this, &SongLibraryPanel::itemDoubleClicked);
    connect(m_impl->songTree, &QTreeWidget::customContextMenuRequested, this, &SongLibraryPanel::showContextMenu);

    // SongLibrary's mutation signals fire AFTER the underlying data has
    // already changed (unlike the *Collection classes' will/was-added
    // pairs), so a full rebuild on any change is the simplest correct
    // response - refreshTree() preserves selection/expansion across it.
    connect(library, &SongLibrary::songAdded, this, [this](qint64){ refreshTree(); });
    connect(library, &SongLibrary::songRemoved, this, [this](qint64){ refreshTree(); });
    connect(library, &SongLibrary::songUpdated, this, [this](qint64){ refreshTree(); });
    connect(library, &SongLibrary::sequenceAdded, this, [this](qint64, qint64){ refreshTree(); });
    connect(library, &SongLibrary::sequenceRemoved, this, [this](qint64, qint64){ refreshTree(); });
    connect(library, &SongLibrary::defaultSequenceChanged, this, [this](qint64, qint64){ refreshTree(); });
    // open()/close() (e.g. the library path being set/changed in Settings
    // while this panel is already open) load/clear the whole catalog
    // without going through the signals above - needs the same rebuild.
    connect(library, &SongLibrary::opened, this, &SongLibraryPanel::refreshTree);
    connect(library, &SongLibrary::closed, this, &SongLibraryPanel::refreshTree);

    refreshTree();
}

SongLibraryPanel::~SongLibraryPanel()
{
    // The capture process isn't QObject-parented to this panel, and its
    // completed() handler captures [this] - stop it and disconnect before the
    // panel goes away, so a signal arriving later can't run that lambda
    // against a dangling this (mirrors SequenceWidget's ~SequenceWidget()).
    if(m_impl->captureProcess)
    {
        m_impl->captureProcess->disconnect(this);
        m_impl->captureProcess->stopCapture();
        delete m_impl->captureProcess;
    }
    delete m_impl->captureSequence;

    delete m_impl;
}

void SongLibraryPanel::openLibraryPrompt()
{
    QMessageBox::information(this, "Song Library",
        "No song library is configured yet. Set a folder for it in Settings > General, "
        "then reopen this panel.");
    SettingsDialog dialog;
    dialog.exec();
}

void SongLibraryPanel::refreshTree()
{
    // Preserve selection and which songs are expanded across the rebuild,
    // so a routine change (e.g. a VDJ import completing) doesn't visibly
    // reset the user's place in the list.
    qint64 selectedSongId = -1;
    qint64 selectedSequenceId = -1;
    if(auto *current = m_impl->songTree->currentItem())
    {
        if(current->data(0, KindRole).toInt() == SongItem)
            selectedSongId = current->data(0, IdRole).toLongLong();
        else
        {
            selectedSongId = current->data(0, SongIdRole).toLongLong();
            selectedSequenceId = current->data(0, IdRole).toLongLong();
        }
    }

    QSet<qint64> expandedSongIds;
    for(int i = 0; i < m_impl->songTree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = m_impl->songTree->topLevelItem(i);
        if(item->isExpanded())
            expandedSongIds.insert(item->data(0, IdRole).toLongLong());
    }

    m_impl->songTree->clear();

    SongLibrary *library = photonApp->songLibrary();
    QTreeWidgetItem *itemToSelect = nullptr;

    for(const auto &song : library->songs())
    {
        auto *songItem = new QTreeWidgetItem(m_impl->songTree);
        songItem->setText(0, songLabel(song));
        songItem->setData(0, IdRole, song.id);
        songItem->setData(0, KindRole, SongItem);

        // A song with only one sequence has nothing to disambiguate - the
        // song row itself stands in for it (double-click opens it directly).
        // Children only appear once there's an actual choice to show.
        if(song.sequences.size() > 1)
        {
            for(const auto &seq : song.sequences)
            {
                auto *seqItem = new QTreeWidgetItem(songItem);
                seqItem->setText(0, sequenceLabel(seq));
                seqItem->setData(0, IdRole, seq.id);
                seqItem->setData(0, KindRole, SequenceItem);
                seqItem->setData(0, SongIdRole, song.id);

                if(selectedSongId == song.id && selectedSequenceId == seq.id)
                    itemToSelect = seqItem;
            }

            if(expandedSongIds.contains(song.id))
                songItem->setExpanded(true);
        }

        if(selectedSongId == song.id && selectedSequenceId < 0)
            itemToSelect = songItem;
    }

    if(itemToSelect)
        m_impl->songTree->setCurrentItem(itemToSelect);
}

void SongLibraryPanel::importVdjClicked()
{
    if(m_impl->captureProcess)
        return;   // already capturing; the button is disabled meanwhile anyway

    SongLibrary *library = photonApp->songLibrary();
    if(!library->isOpen())
    {
        openLibraryPrompt();
        return;
    }

    VirtualDJConnector *connector = photonApp->djConnector();
    if(!connector || !connector->isConnected())
    {
        QMessageBox::warning(this, "Song Library",
            "VirtualDJ isn't connected. Make sure VirtualDJ is running with the "
            "Photon connector plugin enabled, then try again.");
        return;
    }

    if(connector->title.isEmpty() && connector->artist.isEmpty())
    {
        QMessageBox::warning(this, "Song Library", "VirtualDJ isn't reporting a current track yet.");
        return;
    }

    // Snapshot the path ourselves - VirtualDJCaptureProcess deliberately leaves
    // SongData::sourcePath blank (the streaming-track case) and doesn't expose
    // its own start-of-capture snapshot.
    m_impl->captureSourcePath = connector->path;

    // A scratch carrier, never added to any collection - VirtualDJCaptureProcess
    // only knows how to write into a Sequence's SongData, so it needs one, but
    // the library only wants the resulting SongData, not a new Sequence.
    m_impl->captureSequence = new Sequence("VDJ Import");

    m_impl->captureProcess = new VirtualDJCaptureProcess;
    m_impl->captureProcess->init(m_impl->captureSequence);

    connect(m_impl->captureProcess, &AudioProcessor::completed, this, [this](){
        SongData *songData = m_impl->captureSequence->songData();
        SongLibrary *library = photonApp->songLibrary();

        if(!songData->isEmpty())
        {
            SongLibraryEntry *entry = library->addOrUpdateFromVdj(
                songData->title(), songData->artist(), songData->duration(), m_impl->captureSourcePath);
            if(entry)
                library->saveSongData(*entry, *songData);
        }

        m_impl->importVdjButton->setEnabled(true);
        m_impl->cancelImportButton->setVisible(false);
        m_impl->importProgress->setVisible(false);
        m_impl->importRemainingLabel->setVisible(false);

        m_impl->captureProcess->deleteLater();
        m_impl->captureProcess = nullptr;
        delete m_impl->captureSequence;
        m_impl->captureSequence = nullptr;
    });

    m_impl->importVdjButton->setEnabled(false);
    m_impl->cancelImportButton->setVisible(true);
    m_impl->importProgress->setVisible(true);
    m_impl->importProgress->setRange(0, 0);   // indeterminate until songLength is known
    m_impl->importRemainingLabel->setVisible(true);
    m_impl->importRemainingLabel->setText("Starting...");

    m_impl->captureProcess->startProcessing();
}

void SongLibraryPanel::cancelImportClicked()
{
    if(m_impl->captureProcess)
        m_impl->captureProcess->stopCapture();
}

void SongLibraryPanel::importTick()
{
    if(!m_impl->captureProcess)
        return;

    VirtualDJConnector *connector = photonApp->djConnector();
    if(connector->songLength > 0.0)
    {
        m_impl->importProgress->setRange(0, int(connector->songLength));
        m_impl->importProgress->setValue(int(connector->time));
        m_impl->importRemainingLabel->setText(formatRemaining(connector->songLength - connector->time));
    }
}

void SongLibraryPanel::setDefaultClicked()
{
    auto *item = m_impl->songTree->currentItem();
    if(!item || item->data(0, KindRole).toInt() != SequenceItem)
        return;

    const qint64 songId = item->data(0, SongIdRole).toLongLong();
    const qint64 sequenceId = item->data(0, IdRole).toLongLong();
    photonApp->songLibrary()->setDefaultSequence(songId, sequenceId);
}

void SongLibraryPanel::newSequenceClicked()
{
    auto *item = m_impl->songTree->currentItem();
    if(!item || item->data(0, KindRole).toInt() != SongItem)
        return;

    SongLibrary *library = photonApp->songLibrary();
    const qint64 songId = item->data(0, IdRole).toLongLong();

    // Resolved once up front just to default the name prompt to the song's own
    // title (per-editor request: a sequence created for a library song should be
    // named after it) - re-resolved below after the modal closes regardless.
    SongLibraryEntry *song = library->findSongById(songId);
    if(!song)
        return;

    bool ok;
    QString name = QInputDialog::getText(this, "New Sequence", "Name:", QLineEdit::Normal,
                                         song->title, &ok);
    if(!ok || name.isEmpty())
        return;

    // Re-resolve after the modal dialog - a VDJ import completing while it was open
    // could have reallocated the library's song vector (see the pointer-lifetime
    // warning on SongLibrary's find/add methods).
    song = library->findSongById(songId);
    if(!song)
        return;

    QDir dir(library->libraryPath());
    dir.mkpath("sequences");
    const QString path = dir.filePath("sequences/" + sanitizedFileStem(name) + ".seq");

    Sequence *sequence = new Sequence;
    sequence->setName(name);
    sequence->setIsLibrarySequence(true);
    sequence->init();
    // Seed the new sequence's SongData from the library's catalog entry (identity,
    // plus any captured beat/feature analysis) rather than leaving it empty.
    *sequence->songData() = library->songDataFor(*song);

    // Link before the first save() - Sequence::save() checks for a library link
    // to decide whether to write SongData through the library (no redundant local
    // sidecar) or fall back to one; linking after saving would miss that on this
    // very first save.
    library->addSequence(songId, path, name);

    sequence->save(path);

    photonApp->sequences()->addSequence(sequence);
    photonApp->sequences()->editSequence(sequence);
}

void SongLibraryPanel::removeClicked()
{
    auto *item = m_impl->songTree->currentItem();
    if(!item)
        return;

    if(item->data(0, KindRole).toInt() == SongItem)
    {
        if(QMessageBox::question(this, "Remove Song",
               "Remove this song from the library? Its sequence files are not deleted.")
           != QMessageBox::Yes)
            return;

        photonApp->songLibrary()->removeSong(item->data(0, IdRole).toLongLong());
    }
    else
    {
        const qint64 songId = item->data(0, SongIdRole).toLongLong();
        const qint64 sequenceId = item->data(0, IdRole).toLongLong();
        photonApp->songLibrary()->removeSequence(songId, sequenceId);
    }
}

void SongLibraryPanel::itemDoubleClicked(QTreeWidgetItem *t_item)
{
    if(!t_item)
        return;

    qint64 songId = -1;
    qint64 sequenceId = -1;

    if(t_item->data(0, KindRole).toInt() == SongItem)
    {
        songId = t_item->data(0, IdRole).toLongLong();
        SongLibraryEntry *song = photonApp->songLibrary()->findSongById(songId);
        // >1 sequences has nothing unambiguous to open here - the tree's own
        // default double-click behavior (expand/collapse) handles that case.
        // 0 sequences has nothing to open at all.
        if(!song || song->sequences.size() != 1)
            return;
        sequenceId = song->sequences.first().id;
    }
    else
    {
        songId = t_item->data(0, SongIdRole).toLongLong();
        sequenceId = t_item->data(0, IdRole).toLongLong();
    }

    SongLibraryEntry *song = photonApp->songLibrary()->findSongById(songId);
    if(!song)
        return;

    for(const auto &seq : song->sequences)
    {
        if(seq.id == sequenceId)
        {
            photonApp->loadSequence(seq.filePath);
            break;
        }
    }
}

void SongLibraryPanel::showContextMenu(const QPoint &t_pos)
{
    QTreeWidgetItem *item = m_impl->songTree->itemAt(t_pos);
    if(!item)
        return;

    m_impl->songTree->setCurrentItem(item);

    QMenu menu(this);
    if(item->data(0, KindRole).toInt() == SequenceItem)
    {
        menu.addAction("Set as Default", this, &SongLibraryPanel::setDefaultClicked);
        menu.addAction("Remove", this, &SongLibraryPanel::removeClicked);
    }
    else
    {
        menu.addAction("New Sequence...", this, &SongLibraryPanel::newSequenceClicked);
        menu.addAction("Remove", this, &SongLibraryPanel::removeClicked);
    }

    menu.exec(m_impl->songTree->viewport()->mapToGlobal(t_pos));
}

} // namespace photon

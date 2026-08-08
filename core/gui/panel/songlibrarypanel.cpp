#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
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

SongLibraryModel::SongLibraryModel(SongLibrary *t_library) : QAbstractListModel(), m_library(t_library)
{
    connect(m_library, &SongLibrary::songAdded, this, &SongLibraryModel::reset);
    connect(m_library, &SongLibrary::songRemoved, this, &SongLibraryModel::reset);
    connect(m_library, &SongLibrary::songUpdated, this, &SongLibraryModel::reset);
    // open()/close() (e.g. the library path being set/changed in Settings while
    // this panel is already open) load/clear the whole catalog without going
    // through songAdded/songRemoved - needs the same reset.
    connect(m_library, &SongLibrary::opened, this, &SongLibraryModel::reset);
    connect(m_library, &SongLibrary::closed, this, &SongLibraryModel::reset);
}

void SongLibraryModel::reset()
{
    beginResetModel();
    endResetModel();
}

int SongLibraryModel::rowCount(const QModelIndex &) const
{
    return m_library->songCount();
}

QVariant SongLibraryModel::data(const QModelIndex &t_index, int t_role) const
{
    if(!t_index.isValid() || t_index.row() < 0 || t_index.row() >= m_library->songCount())
        return QVariant();

    if(t_role != Qt::DisplayRole)
        return QVariant();

    const SongLibraryEntry &entry = m_library->songs().at(t_index.row());
    return entry.title + " - " + entry.artist;
}

qint64 SongLibraryModel::idAt(int t_row) const
{
    if(t_row < 0 || t_row >= m_library->songCount())
        return -1;
    return m_library->songs().at(t_row).id;
}

namespace {

// Small modal dialog for the identity fields a locally-added song needs.
// Duration is entered by hand for now - Photon doesn't yet probe it from the
// file automatically (see the song-analysis-model notes on the Vamp pipeline).
bool promptSongDetails(QWidget *t_parent, const QString &t_defaultTitle, QString &outTitle,
                       QString &outArtist, double &outDuration)
{
    QDialog dialog(t_parent);
    dialog.setWindowTitle("Song Details");

    QFormLayout *form = new QFormLayout;
    auto *titleEdit = new QLineEdit(t_defaultTitle);
    auto *artistEdit = new QLineEdit;
    auto *durationSpin = new QDoubleSpinBox;
    durationSpin->setRange(0.0, 24.0 * 60.0 * 60.0);
    durationSpin->setDecimals(1);
    durationSpin->setSuffix(" s");

    form->addRow("Title", titleEdit);
    form->addRow("Artist", artistEdit);
    form->addRow("Duration", durationSpin);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if(dialog.exec() != QDialog::Accepted || titleEdit->text().isEmpty())
        return false;

    outTitle = titleEdit->text();
    outArtist = artistEdit->text();
    outDuration = durationSpin->value();
    return true;
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

    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // --- left: songs ---
    QWidget *leftWidget = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    m_impl->model = new SongLibraryModel(library);
    m_impl->songList = new QListView;
    m_impl->songList->setModel(m_impl->model);
    m_impl->songList->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    leftLayout->addWidget(m_impl->songList);

    m_impl->addLocalButton = new QPushButton("Add Local Song...");
    m_impl->importVdjButton = new QPushButton("Import VDJ Song");
    m_impl->cancelImportButton = new QPushButton("Cancel");
    m_impl->cancelImportButton->setVisible(false);
    m_impl->removeSongButton = new QPushButton("Remove");
    m_impl->removeSongButton->setEnabled(false);

    QHBoxLayout *songButtons = new QHBoxLayout;
    songButtons->addWidget(m_impl->addLocalButton);
    songButtons->addWidget(m_impl->importVdjButton);
    songButtons->addWidget(m_impl->cancelImportButton);
    songButtons->addWidget(m_impl->removeSongButton);
    leftLayout->addLayout(songButtons);

    m_impl->importProgress = new QProgressBar;
    m_impl->importProgress->setVisible(false);
    m_impl->importProgress->setTextVisible(false);
    leftLayout->addWidget(m_impl->importProgress);

    m_impl->importRemainingLabel = new QLabel;
    m_impl->importRemainingLabel->setVisible(false);
    leftLayout->addWidget(m_impl->importRemainingLabel);

    splitter->addWidget(leftWidget);

    // --- right: selected song + its sequences ---
    QWidget *rightWidget = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);

    QFormLayout *detailsForm = new QFormLayout;
    m_impl->titleLabel = new QLabel;
    m_impl->artistLabel = new QLabel;
    m_impl->durationLabel = new QLabel;
    m_impl->sourceLabel = new QLabel;
    detailsForm->addRow("Title", m_impl->titleLabel);
    detailsForm->addRow("Artist", m_impl->artistLabel);
    detailsForm->addRow("Duration", m_impl->durationLabel);
    detailsForm->addRow("Source", m_impl->sourceLabel);
    rightLayout->addLayout(detailsForm);

    m_impl->sequenceList = new QListWidget;
    m_impl->sequenceList->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    rightLayout->addWidget(m_impl->sequenceList);

    m_impl->setDefaultButton = new QPushButton("Set Default");
    m_impl->addExistingButton = new QPushButton("Add Existing...");
    m_impl->newSequenceButton = new QPushButton("New Sequence...");
    m_impl->removeSequenceButton = new QPushButton("Remove");

    QHBoxLayout *sequenceButtons = new QHBoxLayout;
    sequenceButtons->addWidget(m_impl->setDefaultButton);
    sequenceButtons->addWidget(m_impl->addExistingButton);
    sequenceButtons->addWidget(m_impl->newSequenceButton);
    sequenceButtons->addWidget(m_impl->removeSequenceButton);
    rightLayout->addLayout(sequenceButtons);

    splitter->addWidget(rightWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitter);
    setPanelLayout(mainLayout);

    connect(m_impl->songList->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](){
        songSelectionChanged();
    });
    connect(m_impl->addLocalButton, &QPushButton::clicked, this, &SongLibraryPanel::addLocalClicked);
    connect(m_impl->importVdjButton, &QPushButton::clicked, this, &SongLibraryPanel::importVdjClicked);
    connect(m_impl->cancelImportButton, &QPushButton::clicked, this, &SongLibraryPanel::cancelImportClicked);
    connect(photonApp->timekeeper(), &Timekeeper::tick, this, &SongLibraryPanel::importTick);
    connect(m_impl->removeSongButton, &QPushButton::clicked, this, &SongLibraryPanel::removeSongClicked);
    connect(m_impl->setDefaultButton, &QPushButton::clicked, this, &SongLibraryPanel::setDefaultClicked);
    connect(m_impl->addExistingButton, &QPushButton::clicked, this, &SongLibraryPanel::addExistingClicked);
    connect(m_impl->newSequenceButton, &QPushButton::clicked, this, &SongLibraryPanel::newSequenceClicked);
    connect(m_impl->removeSequenceButton, &QPushButton::clicked, this, &SongLibraryPanel::removeSequenceClicked);
    connect(m_impl->sequenceList, &QListWidget::itemDoubleClicked, this, &SongLibraryPanel::sequenceDoubleClicked);
    connect(m_impl->sequenceList, &QListWidget::currentItemChanged, this, [this](){
        const bool hasSequenceSelection = m_impl->sequenceList->currentItem() != nullptr;
        m_impl->setDefaultButton->setEnabled(hasSequenceSelection);
        m_impl->removeSequenceButton->setEnabled(hasSequenceSelection);
    });

    // The library can open/close out from under this panel (e.g. the path is
    // set/changed in Settings while this panel is already open) - the
    // previously-selected song id may no longer resolve, so drop the
    // selection rather than show stale details.
    connect(library, &SongLibrary::opened, this, [this](){
        m_impl->selectedSongId = -1;
        refreshSelectedSong();
    });
    connect(library, &SongLibrary::closed, this, [this](){
        m_impl->selectedSongId = -1;
        refreshSelectedSong();
    });

    refreshSelectedSong();
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

void SongLibraryPanel::songSelectionChanged()
{
    const QModelIndex index = m_impl->songList->currentIndex();
    m_impl->selectedSongId = index.isValid() ? m_impl->model->idAt(index.row()) : -1;
    refreshSelectedSong();
}

void SongLibraryPanel::refreshSelectedSong()
{
    SongLibrary *library = photonApp->songLibrary();
    SongLibraryEntry *song = m_impl->selectedSongId >= 0 ? library->findSongById(m_impl->selectedSongId) : nullptr;

    m_impl->removeSongButton->setEnabled(song != nullptr);
    m_impl->addExistingButton->setEnabled(song != nullptr);
    m_impl->newSequenceButton->setEnabled(song != nullptr);

    m_impl->sequenceList->clear();

    if(!song)
    {
        m_impl->titleLabel->setText(QString());
        m_impl->artistLabel->setText(QString());
        m_impl->durationLabel->setText(QString());
        m_impl->sourceLabel->setText(QString());
        m_impl->setDefaultButton->setEnabled(false);
        m_impl->removeSequenceButton->setEnabled(false);
        return;
    }

    m_impl->titleLabel->setText(song->title);
    m_impl->artistLabel->setText(song->artist);
    m_impl->durationLabel->setText(QString::number(song->duration, 'f', 1) + " s");
    m_impl->sourceLabel->setText(QString::fromUtf8(song->source));

    for(const auto &seq : song->sequences)
    {
        auto *item = new QListWidgetItem(seq.isDefault ? seq.name + "  (Default)" : seq.name);
        item->setData(Qt::UserRole, QVariant::fromValue(seq.id));
        m_impl->sequenceList->addItem(item);
    }

    const bool hasSequenceSelection = m_impl->sequenceList->currentItem() != nullptr;
    m_impl->setDefaultButton->setEnabled(hasSequenceSelection);
    m_impl->removeSequenceButton->setEnabled(hasSequenceSelection);
}

void SongLibraryPanel::addLocalClicked()
{
    SongLibrary *library = photonApp->songLibrary();
    if(!library->isOpen())
    {
        openLibraryPrompt();
        return;
    }

    const QString audioPath = QFileDialog::getOpenFileName(this, "Add Local Song", QString(),
        "Audio Files (*.mp3 *.wav *.flac *.m4a *.aac *.ogg);;All Files (*)");
    if(audioPath.isEmpty())
        return;

    QString title, artist;
    double duration = 0.0;
    if(!promptSongDetails(this, QFileInfo(audioPath).completeBaseName(), title, artist, duration))
        return;

    library->addLocalSong(audioPath, title, artist, duration);
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

void SongLibraryPanel::removeSongClicked()
{
    if(m_impl->selectedSongId < 0)
        return;

    if(QMessageBox::question(this, "Remove Song",
           "Remove this song from the library? Its sequence files are not deleted.")
       != QMessageBox::Yes)
        return;

    photonApp->songLibrary()->removeSong(m_impl->selectedSongId);
    m_impl->selectedSongId = -1;
    refreshSelectedSong();
}

void SongLibraryPanel::setDefaultClicked()
{
    auto *item = m_impl->sequenceList->currentItem();
    if(!item || m_impl->selectedSongId < 0)
        return;

    const qint64 sequenceId = item->data(Qt::UserRole).toLongLong();
    photonApp->songLibrary()->setDefaultSequence(m_impl->selectedSongId, sequenceId);
    refreshSelectedSong();
}

void SongLibraryPanel::addExistingClicked()
{
    if(m_impl->selectedSongId < 0)
        return;

    const QString path = QFileDialog::getOpenFileName(this, "Add Existing Sequence", QString(),
        "Photon Sequence (*.seq)");
    if(path.isEmpty())
        return;

    bool ok;
    QString name = QInputDialog::getText(this, "Sequence Name", "Name:", QLineEdit::Normal,
                                         QFileInfo(path).completeBaseName(), &ok);
    if(!ok || name.isEmpty())
        return;

    photonApp->songLibrary()->addSequence(m_impl->selectedSongId, path, name);
    refreshSelectedSong();
}

void SongLibraryPanel::newSequenceClicked()
{
    SongLibrary *library = photonApp->songLibrary();
    if(m_impl->selectedSongId < 0)
        return;

    // Resolved once up front just to default the name prompt to the song's own
    // title (per-editor request: a sequence created for a library song should be
    // named after it) - re-resolved below after the modal closes regardless.
    SongLibraryEntry *song = library->findSongById(m_impl->selectedSongId);
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
    song = library->findSongById(m_impl->selectedSongId);
    if(!song)
        return;

    QDir dir(library->libraryPath());
    dir.mkpath("sequences");
    const QString path = dir.filePath("sequences/" + sanitizedFileStem(name) + ".seq");

    Sequence *sequence = new Sequence;
    sequence->setName(name);
    // Seed the new sequence's SongData from the library's catalog entry (identity,
    // plus any captured beat/feature analysis) rather than leaving it empty.
    *sequence->songData() = library->songDataFor(*song);

    // Link before the first save() - Sequence::save() checks for a library link
    // to decide whether to write SongData through the library (no redundant local
    // sidecar) or fall back to one; linking after saving would miss that on this
    // very first save.
    library->addSequence(m_impl->selectedSongId, path, name);

    sequence->save(path);

    photonApp->sequences()->addSequence(sequence);
    photonApp->sequences()->editSequence(sequence);

    refreshSelectedSong();
}

void SongLibraryPanel::removeSequenceClicked()
{
    auto *item = m_impl->sequenceList->currentItem();
    if(!item || m_impl->selectedSongId < 0)
        return;

    const qint64 sequenceId = item->data(Qt::UserRole).toLongLong();
    photonApp->songLibrary()->removeSequence(m_impl->selectedSongId, sequenceId);
    refreshSelectedSong();
}

void SongLibraryPanel::sequenceDoubleClicked(QListWidgetItem *t_item)
{
    if(!t_item || m_impl->selectedSongId < 0)
        return;

    SongLibraryEntry *song = photonApp->songLibrary()->findSongById(m_impl->selectedSongId);
    if(!song)
        return;

    const qint64 sequenceId = t_item->data(Qt::UserRole).toLongLong();
    for(const auto &seq : song->sequences)
    {
        if(seq.id == sequenceId)
        {
            photonApp->loadSequence(seq.filePath);
            break;
        }
    }
}

} // namespace photon

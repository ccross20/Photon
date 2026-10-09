#include <cmath>
#include <QMutexLocker>
#include "virtualdjplayback.h"
#include "virtualdjconnector.h"
#include "audio/songdata.h"
#include "library/songlibrary.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"

namespace photon {

namespace {

// Reported lengths for the same track can differ slightly between a local
// file's analysis and VirtualDJ's own; beyond this they're different edits.
constexpr double kDurationTolerance = 2.0;

// No position change for this long means VirtualDJ is paused.
constexpr qint64 kPausedAfterMs = 300;
// Never extrapolate further than this past the last telemetry tick.
constexpr qint64 kMaxExtrapolationMs = 500;
// No telemetry for this long means VirtualDJ is gone; stop driving lights.
constexpr qint64 kStaleAfterMs = 2000;

QString normalized(const QString &t_text)
{
    return t_text.trimmed().toLower();
}

const SongLibrarySequenceEntry *preferredSequence(const SongLibraryEntry &t_song)
{
    for(const auto &sequence : t_song.sequences)
        if(sequence.isDefault)
            return &sequence;
    return t_song.sequences.isEmpty() ? nullptr : &t_song.sequences.first();
}

} // namespace

VirtualDJPlayback::VirtualDJPlayback(VirtualDJConnector *t_connector, SongLibrary *t_library,
                                     SequenceCollection *t_sequences, QObject *t_parent)
    : QObject(t_parent), m_connector(t_connector), m_library(t_library), m_sequences(t_sequences)
{
    m_clock.start();
    connect(m_connector, &VirtualDJConnector::dataUpdated, this, &VirtualDJPlayback::dataUpdated);
    connect(m_sequences, &SequenceCollection::sequenceWillBeRemoved, this, &VirtualDJPlayback::sequenceWillBeRemoved);
    m_status = "Waiting for VirtualDJ";
}

VirtualDJPlayback::~VirtualDJPlayback()
{
    QMutexLocker lock(&m_mutex);
    m_sequence = nullptr;
}

bool VirtualDJPlayback::isPlaying() const
{
    QMutexLocker lock(&m_mutex);
    return m_playing && m_clock.elapsed() - m_receivedMs <= kStaleAfterMs;
}

bool VirtualDJPlayback::hasSequence() const
{
    // Only while VirtualDJ is still there: the match from before a lost
    // connection (or a VirtualDJ that stopped sending) doesn't count.
    if(!m_connector->isConnected())
        return false;
    QMutexLocker lock(&m_mutex);
    return m_sequence != nullptr && m_clock.elapsed() - m_receivedMs <= kStaleAfterMs;
}

bool VirtualDJPlayback::process(const std::function<void(Sequence *, double)> &t_fn) const
{
    QMutexLocker lock(&m_mutex);
    if(!m_sequence)
        return false;

    const qint64 sinceTick = m_clock.elapsed() - m_receivedMs;
    if(sinceTick > kStaleAfterMs)
        return false;

    double songTime = m_songTime;
    if(m_playing)
        songTime += m_rate * std::min(sinceTick, kMaxExtrapolationMs) / 1000.0;

    t_fn(m_sequence, songTime);
    return true;
}

QString VirtualDJPlayback::status() const
{
    return m_status;
}

void VirtualDJPlayback::setStatus(const QString &t_status)
{
    if(m_status == t_status)
        return;
    m_status = t_status;
    qWarning().noquote() << "VirtualDJPlayback:" << t_status;
    emit statusChanged(t_status);
}

void VirtualDJPlayback::dataUpdated()
{
    const QString identity = m_connector->path + "|" + normalized(m_connector->artist) + "|"
                             + normalized(m_connector->title) + "|"
                             + QString::number(std::llround(m_connector->songLength));
    if(identity != m_trackIdentity)
    {
        m_trackIdentity = identity;
        matchTrack();
    }

    const qint64 now = m_clock.elapsed();
    const double time = m_connector->time;

    QMutexLocker lock(&m_mutex);
    if(time != m_songTime)
    {
        // Track the real playback rate (pitch/tempo changes) between ticks,
        // ignoring seeks and track loads, which look like huge rates.
        const double wallSeconds = (now - m_receivedMs) / 1000.0;
        if(m_playing && wallSeconds > 0.0)
        {
            const double measured = (time - m_songTime) / wallSeconds;
            if(measured > 0.5 && measured < 2.0)
                m_rate += (measured - m_rate) * 0.2;
        }
        m_playing = true;
        m_lastMoveMs = now;
    }
    else if(now - m_lastMoveMs > kPausedAfterMs)
    {
        m_playing = false;
    }
    m_songTime = time;
    m_receivedMs = now;
}

void VirtualDJPlayback::matchTrack()
{
    const QString title = m_connector->title;
    const QString artist = m_connector->artist;
    const double length = m_connector->songLength;
    const QString trackName = artist.isEmpty() ? title : artist + " - " + title;

    if(title.isEmpty() && m_connector->path.isEmpty())
    {
        setSequence(nullptr, false);
        setStatus("No track loaded in VirtualDJ");
        return;
    }
    if(!m_library || !m_library->isOpen())
    {
        setSequence(nullptr, false);
        setStatus("Song Library is not open");
        return;
    }

    const SongLibraryEntry *song = m_library->findSongByTrackKey(SongData::makeTrackKey(artist, title, length));

    if(!song)
    {
        double bestDelta = kDurationTolerance;
        for(const auto &candidate : m_library->songs())
        {
            if(normalized(candidate.title) != normalized(title) || normalized(candidate.artist) != normalized(artist))
                continue;
            const double delta = std::abs(candidate.duration - length);
            if(delta <= bestDelta)
            {
                bestDelta = delta;
                song = &candidate;
            }
        }
    }

    if(!song && !m_connector->path.isEmpty())
    {
        for(const auto &candidate : m_library->songs())
        {
            if(candidate.sourcePath == m_connector->path)
            {
                song = &candidate;
                break;
            }
        }
    }

    const SongLibrarySequenceEntry *entry = song ? preferredSequence(*song) : nullptr;
    if(!entry)
    {
        setSequence(nullptr, false);
        setStatus(song ? "No sequence for " + trackName : "Not in Song Library: " + trackName);
        return;
    }

    // Reuse a copy that's already loaded (e.g. open in the editor) so edits
    // show up live; otherwise load one just for playback.
    const QString sequencePath = entry->filePath;
    const QString sequenceName = entry->name;
    for(Sequence *existing : m_sequences->sequences())
    {
        if(existing->filePath() == sequencePath)
        {
            setSequence(existing, false);
            setStatus("Playing " + sequenceName + " for " + trackName);
            return;
        }
    }

    auto *sequence = new Sequence;
    sequence->setIsLibrarySequence(true);
    sequence->load(sequencePath);
    m_sequences->addSequence(sequence);
    setSequence(sequence, true);
    setStatus("Playing " + sequenceName + " for " + trackName);
}

void VirtualDJPlayback::setSequence(Sequence *t_sequence, bool t_loadedHere)
{
    Sequence *previous = nullptr;
    bool previousLoadedHere = false;
    {
        QMutexLocker lock(&m_mutex);
        if(m_sequence == t_sequence)
            return;
        previous = m_sequence;
        previousLoadedHere = m_loadedHere;
        m_sequence = t_sequence;
        m_loadedHere = t_loadedHere;
        m_playing = false;
        m_rate = 1.0;
    }

    // Only unload what this class loaded itself, and not if the user has
    // since opened it in an editor.
    if(previous && previousLoadedHere && !m_sequences->panelFor(previous))
        m_sequences->removeSequence(previous);
}

void VirtualDJPlayback::sequenceWillBeRemoved(Sequence *t_sequence)
{
    {
        // Blocks until any in-flight process() finishes with it.
        QMutexLocker lock(&m_mutex);
        if(m_sequence != t_sequence)
            return;
        m_sequence = nullptr;
        m_loadedHere = false;
    }
    setStatus("Sequence was closed");
}

} // namespace photon

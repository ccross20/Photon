#ifndef PHOTON_SONGLIBRARY_H
#define PHOTON_SONGLIBRARY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVector>
#include "photon-global.h"

namespace photon {

class SongData;

// One sequence built for a library song. filePath is a normal .seq file
// (wherever it lives) - the library only tracks the association, it doesn't
// own or move the sequence file itself.
struct PHOTONCORE_EXPORT SongLibrarySequenceEntry
{
    qint64 id = -1;
    qint64 songId = -1;
    QString filePath;
    QString name;
    bool isDefault = false;
};

// One catalogued song. trackKey mirrors SongData::trackKey() (see songdata.h) -
// explicit when known, else derived from artist|title|duration - so the future
// VirtualDJ sync node can look a now-playing track up by the same key it
// already knows how to compute.
struct PHOTONCORE_EXPORT SongLibraryEntry
{
    qint64 id = -1;
    QByteArray trackKey;
    QString title;
    QString artist;
    double duration = 0.0;
    QByteArray source;        // "local" or "virtualdj"
    QString sourcePath;       // local file path, or VirtualDJ-reported path
    QVector<SongLibrarySequenceEntry> sequences;
};

// A persistent, SQLite-backed catalog mapping songs (by identity, not by file)
// to the set of sequences built for them, with one marked default. Independent
// of any open Project - this is an app-level library, opened from a path
// configured in Settings (ApplicationSettings::songDataLibraryPath()).
//
// At open() the whole catalog is loaded into memory (hundreds of rows is
// trivial) and kept in sync with SQLite on every mutation - callers read via
// songs()/findSongBy...() and mutate via the add/remove/set methods below,
// mirroring the shape of this codebase's existing *Collection classes (see
// RoutineCollection) even though the storage underneath is a real database,
// not project-file JSON.
class PHOTONCORE_EXPORT SongLibrary : public QObject
{
    Q_OBJECT
public:
    explicit SongLibrary(QObject *parent = nullptr);
    ~SongLibrary();

    // Opens (creating if needed) <libraryPath>/library.sqlite and its schema,
    // then loads the catalog. Safe to call again with a new path (closes the
    // old connection first). Returns false on failure (bad path, DB error).
    bool open(const QString &libraryPath);
    void close();
    bool isOpen() const;
    QString libraryPath() const;

    int songCount() const;
    const QVector<SongLibraryEntry> &songs() const;

    // These, and every add*() below, return a pointer into the internal
    // QVector<SongLibraryEntry> - valid only until the next call that adds or
    // removes a song (which can reallocate the vector). Read what you need
    // immediately; don't hold the pointer across other calls or event-loop
    // turns - keep the entry's `id` instead and re-resolve via findSongById()
    // when you need it again.
    SongLibraryEntry *findSongByTrackKey(const QByteArray &trackKey);
    SongLibraryEntry *findSongById(qint64 songId);
    // Finds the song (if any) that has a sequence linked at exactly this file
    // path - lets a Sequence discover its own library association from just
    // the path it's being saved/loaded from. Same pointer-lifetime caveat.
    SongLibraryEntry *findSongBySequencePath(const QString &sequenceFilePath);

    // Adds a song from a local audio file. Deduplicates by trackKey - if a
    // song with that key already exists (same artist/title/duration, or an
    // explicit key match), returns the existing entry instead of duplicating.
    SongLibraryEntry *addLocalSong(const QString &audioFilePath, const QString &title,
                                   const QString &artist, double duration);
    // Adds or updates a song from live VirtualDJ telemetry. Same dedup-by-key
    // behaviour as addLocalSong(), so importing the same track twice updates
    // its metadata in place rather than creating a duplicate.
    SongLibraryEntry *addOrUpdateFromVdj(const QString &title, const QString &artist,
                                         double duration, const QString &sourcePath);
    void removeSong(qint64 songId);

    // First sequence added for a song automatically becomes its default.
    // Same pointer-lifetime caveat as findSongById() above - a
    // SongLibrarySequenceEntry* lives inside its song's own entry, so it's
    // invalidated by any later song OR sequence mutation, not just this
    // song's own.
    SongLibrarySequenceEntry *addSequence(qint64 songId, const QString &sequenceFilePath,
                                          const QString &name);
    // If the removed sequence was the default and others remain, the first
    // remaining one is promoted.
    void removeSequence(qint64 songId, qint64 sequenceId);
    void setDefaultSequence(qint64 songId, qint64 sequenceId);

    // This song's canonical analysis, stored under
    // <libraryPath>/songs/<trackKey>.song (SongData's own binary sidecar
    // format - see core/audio/songdata.h). Returns an identity-populated but
    // otherwise empty SongData if no sidecar exists yet.
    SongData songDataFor(const SongLibraryEntry &song) const;
    // Persists (possibly newly-analyzed) SongData back to that same sidecar.
    bool saveSongData(const SongLibraryEntry &song, const SongData &data) const;

signals:
    // Fires at the end of a successful open()/close() - e.g. so a model
    // showing songs() can refresh even though loadAll() itself doesn't go
    // through the add/remove signals below.
    void opened();
    void closed();
    void songAdded(qint64 songId);
    void songRemoved(qint64 songId);
    // Fires when addOrUpdateFromVdj() matches an existing song by trackKey and
    // refreshes its metadata in place, rather than adding a new one.
    void songUpdated(qint64 songId);
    void sequenceAdded(qint64 songId, qint64 sequenceId);
    void sequenceRemoved(qint64 songId, qint64 sequenceId);
    void defaultSequenceChanged(qint64 songId, qint64 sequenceId);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SONGLIBRARY_H

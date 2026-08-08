#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "songlibrary.h"
#include "audio/songdata.h"

namespace photon {

namespace {

qint64 insertSongRow(QSqlDatabase &db, const QByteArray &trackKey, const QString &title,
                     const QString &artist, double duration, const QByteArray &source,
                     const QString &sourcePath)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO songs (track_key, title, artist, duration, source, source_path) "
                 "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(QString::fromLatin1(trackKey));
    query.addBindValue(title);
    query.addBindValue(artist);
    query.addBindValue(duration);
    query.addBindValue(QString::fromLatin1(source));
    query.addBindValue(sourcePath);
    if(!query.exec())
    {
        qWarning() << "SongLibrary: insert song failed" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

bool updateSongRow(QSqlDatabase &db, qint64 id, const QString &title, const QString &artist,
                   double duration, const QByteArray &source, const QString &sourcePath)
{
    QSqlQuery query(db);
    query.prepare("UPDATE songs SET title = ?, artist = ?, duration = ?, source = ?, source_path = ? WHERE id = ?");
    query.addBindValue(title);
    query.addBindValue(artist);
    query.addBindValue(duration);
    query.addBindValue(QString::fromLatin1(source));
    query.addBindValue(sourcePath);
    query.addBindValue(id);
    if(!query.exec())
    {
        qWarning() << "SongLibrary: update song failed" << query.lastError().text();
        return false;
    }
    return true;
}

} // namespace

class SongLibrary::Impl
{
public:
    QString connectionName;   // empty when closed
    QString libraryPath;
    QVector<SongLibraryEntry> songs;

    QString songDataPath(const QByteArray &trackKey) const
    {
        return QDir(libraryPath).filePath("songs/" + QString::fromLatin1(trackKey) + ".song");
    }

    bool ensureSchema(QSqlDatabase &db)
    {
        QSqlQuery pragma(db);
        pragma.exec("PRAGMA foreign_keys = ON");

        QSqlQuery query(db);
        const bool ok =
            query.exec(
                "CREATE TABLE IF NOT EXISTS songs ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "track_key TEXT NOT NULL UNIQUE,"
                "title TEXT NOT NULL,"
                "artist TEXT NOT NULL,"
                "duration REAL NOT NULL DEFAULT 0,"
                "source TEXT NOT NULL DEFAULT '',"
                "source_path TEXT NOT NULL DEFAULT '',"
                "created_at TEXT NOT NULL DEFAULT (datetime('now'))"
                ")")
            && query.exec(
                "CREATE TABLE IF NOT EXISTS sequences ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "song_id INTEGER NOT NULL REFERENCES songs(id) ON DELETE CASCADE,"
                "file_path TEXT NOT NULL,"
                "name TEXT NOT NULL,"
                "is_default INTEGER NOT NULL DEFAULT 0,"
                "created_at TEXT NOT NULL DEFAULT (datetime('now'))"
                ")")
            && query.exec(
                "CREATE UNIQUE INDEX IF NOT EXISTS idx_song_one_default "
                "ON sequences(song_id) WHERE is_default = 1");

        if(!ok)
            qWarning() << "SongLibrary: schema creation failed" << query.lastError().text();

        return ok;
    }

    void loadAll(QSqlDatabase &db)
    {
        songs.clear();

        QHash<qint64, int> indexById;

        QSqlQuery songQuery(db);
        songQuery.exec("SELECT id, track_key, title, artist, duration, source, source_path FROM songs ORDER BY title, artist");
        while(songQuery.next())
        {
            SongLibraryEntry entry;
            entry.id = songQuery.value(0).toLongLong();
            entry.trackKey = songQuery.value(1).toByteArray();
            entry.title = songQuery.value(2).toString();
            entry.artist = songQuery.value(3).toString();
            entry.duration = songQuery.value(4).toDouble();
            entry.source = songQuery.value(5).toByteArray();
            entry.sourcePath = songQuery.value(6).toString();

            indexById.insert(entry.id, songs.size());
            songs.append(entry);
        }

        QSqlQuery seqQuery(db);
        seqQuery.exec("SELECT id, song_id, file_path, name, is_default FROM sequences ORDER BY name");
        while(seqQuery.next())
        {
            SongLibrarySequenceEntry seq;
            seq.id = seqQuery.value(0).toLongLong();
            seq.songId = seqQuery.value(1).toLongLong();
            seq.filePath = seqQuery.value(2).toString();
            seq.name = seqQuery.value(3).toString();
            seq.isDefault = seqQuery.value(4).toInt() != 0;

            auto it = indexById.constFind(seq.songId);
            if(it != indexById.constEnd())
                songs[it.value()].sequences.append(seq);
        }
    }
};

SongLibrary::SongLibrary(QObject *t_parent) : QObject(t_parent), m_impl(new Impl)
{
}

SongLibrary::~SongLibrary()
{
    close();
    delete m_impl;
}

bool SongLibrary::open(const QString &t_libraryPath)
{
    close();

    if(t_libraryPath.isEmpty())
        return false;

    QDir dir(t_libraryPath);
    if(!dir.exists() && !dir.mkpath("."))
    {
        qWarning() << "SongLibrary: could not create library directory" << t_libraryPath;
        return false;
    }
    if(!dir.mkpath("songs"))
    {
        qWarning() << "SongLibrary: could not create songs directory under" << t_libraryPath;
        return false;
    }

    // A uniquely-named connection (not the default one) so multiple opens in
    // the process lifetime, and any other future SQL use, never collide.
    static int s_connectionCounter = 0;
    const QString connectionName = QString("songlibrary_%1").arg(++s_connectionCounter);

    bool opened = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(dir.filePath("library.sqlite"));
        opened = db.open();
        if(!opened)
        {
            qWarning() << "SongLibrary: failed to open" << db.databaseName() << db.lastError().text();
        }
        else
        {
            m_impl->connectionName = connectionName;
            m_impl->libraryPath = t_libraryPath;
            opened = m_impl->ensureSchema(db);
            if(opened)
                m_impl->loadAll(db);
        }
    }   // db (a QSqlDatabase copy) must go out of scope before removeDatabase()

    if(!opened)
    {
        QSqlDatabase::removeDatabase(connectionName);
        m_impl->connectionName.clear();
        m_impl->libraryPath.clear();
        return false;
    }

    emit this->opened();
    return true;
}

void SongLibrary::close()
{
    if(m_impl->connectionName.isEmpty())
        return;

    const QString connectionName = m_impl->connectionName;
    {
        QSqlDatabase db = QSqlDatabase::database(connectionName, false);
        if(db.isOpen())
            db.close();
    }   // release the local QSqlDatabase copy before removeDatabase()
    QSqlDatabase::removeDatabase(connectionName);

    m_impl->connectionName.clear();
    m_impl->libraryPath.clear();
    m_impl->songs.clear();

    emit closed();
}

bool SongLibrary::isOpen() const
{
    return !m_impl->connectionName.isEmpty();
}

QString SongLibrary::libraryPath() const
{
    return m_impl->libraryPath;
}

int SongLibrary::songCount() const
{
    return m_impl->songs.size();
}

const QVector<SongLibraryEntry> &SongLibrary::songs() const
{
    return m_impl->songs;
}

SongLibraryEntry *SongLibrary::findSongByTrackKey(const QByteArray &t_trackKey)
{
    for(auto &entry : m_impl->songs)
        if(entry.trackKey == t_trackKey)
            return &entry;
    return nullptr;
}

SongLibraryEntry *SongLibrary::findSongById(qint64 t_songId)
{
    for(auto &entry : m_impl->songs)
        if(entry.id == t_songId)
            return &entry;
    return nullptr;
}

SongLibraryEntry *SongLibrary::findSongBySequencePath(const QString &t_sequenceFilePath)
{
    if(t_sequenceFilePath.isEmpty())
        return nullptr;

    // Compare absolute paths - callers (e.g. Sequence::save()/load()) and the
    // path stored at link time (e.g. via a file dialog) aren't guaranteed to
    // agree on relative-vs-absolute or slash direction otherwise.
    const QString target = QFileInfo(t_sequenceFilePath).absoluteFilePath();

    for(auto &entry : m_impl->songs)
        for(const auto &seq : entry.sequences)
            if(QFileInfo(seq.filePath).absoluteFilePath() == target)
                return &entry;

    return nullptr;
}

SongLibraryEntry *SongLibrary::addLocalSong(const QString &t_audioFilePath, const QString &t_title,
                                            const QString &t_artist, double t_duration)
{
    if(!isOpen())
        return nullptr;

    const QByteArray key = SongData::makeTrackKey(t_artist, t_title, t_duration);
    if(auto *existing = findSongByTrackKey(key))
        return existing;

    QSqlDatabase db = QSqlDatabase::database(m_impl->connectionName);
    const qint64 id = insertSongRow(db, key, t_title, t_artist, t_duration, "local", t_audioFilePath);
    if(id < 0)
        return nullptr;

    SongLibraryEntry entry;
    entry.id = id;
    entry.trackKey = key;
    entry.title = t_title;
    entry.artist = t_artist;
    entry.duration = t_duration;
    entry.source = "local";
    entry.sourcePath = t_audioFilePath;

    m_impl->songs.append(entry);
    emit songAdded(id);
    return &m_impl->songs.last();
}

SongLibraryEntry *SongLibrary::addOrUpdateFromVdj(const QString &t_title, const QString &t_artist,
                                                  double t_duration, const QString &t_sourcePath)
{
    if(!isOpen())
        return nullptr;

    const QByteArray key = SongData::makeTrackKey(t_artist, t_title, t_duration);
    QSqlDatabase db = QSqlDatabase::database(m_impl->connectionName);

    if(auto *existing = findSongByTrackKey(key))
    {
        if(!updateSongRow(db, existing->id, t_title, t_artist, t_duration, "virtualdj", t_sourcePath))
            return existing;

        existing->title = t_title;
        existing->artist = t_artist;
        existing->duration = t_duration;
        existing->source = "virtualdj";
        existing->sourcePath = t_sourcePath;
        emit songUpdated(existing->id);
        return existing;
    }

    const qint64 id = insertSongRow(db, key, t_title, t_artist, t_duration, "virtualdj", t_sourcePath);
    if(id < 0)
        return nullptr;

    SongLibraryEntry entry;
    entry.id = id;
    entry.trackKey = key;
    entry.title = t_title;
    entry.artist = t_artist;
    entry.duration = t_duration;
    entry.source = "virtualdj";
    entry.sourcePath = t_sourcePath;

    m_impl->songs.append(entry);
    emit songAdded(id);
    return &m_impl->songs.last();
}

void SongLibrary::removeSong(qint64 t_songId)
{
    if(!isOpen())
        return;

    int index = -1;
    for(int i = 0; i < m_impl->songs.size(); ++i)
        if(m_impl->songs[i].id == t_songId) { index = i; break; }
    if(index < 0)
        return;

    const QByteArray key = m_impl->songs[index].trackKey;

    QSqlDatabase db = QSqlDatabase::database(m_impl->connectionName);
    QSqlQuery query(db);
    query.prepare("DELETE FROM songs WHERE id = ?");
    query.addBindValue(t_songId);
    if(!query.exec())
    {
        qWarning() << "SongLibrary: delete song failed" << query.lastError().text();
        return;
    }

    QFile::remove(m_impl->songDataPath(key));   // best-effort; the row is already gone either way

    m_impl->songs.removeAt(index);
    emit songRemoved(t_songId);
}

SongLibrarySequenceEntry *SongLibrary::addSequence(qint64 t_songId, const QString &t_sequenceFilePath,
                                                   const QString &t_name)
{
    if(!isOpen())
        return nullptr;

    SongLibraryEntry *song = findSongById(t_songId);
    if(!song)
        return nullptr;

    const bool makeDefault = song->sequences.isEmpty();

    QSqlDatabase db = QSqlDatabase::database(m_impl->connectionName);
    QSqlQuery query(db);
    query.prepare("INSERT INTO sequences (song_id, file_path, name, is_default) VALUES (?, ?, ?, ?)");
    query.addBindValue(t_songId);
    query.addBindValue(t_sequenceFilePath);
    query.addBindValue(t_name);
    query.addBindValue(makeDefault ? 1 : 0);
    if(!query.exec())
    {
        qWarning() << "SongLibrary: insert sequence failed" << query.lastError().text();
        return nullptr;
    }

    SongLibrarySequenceEntry entry;
    entry.id = query.lastInsertId().toLongLong();
    entry.songId = t_songId;
    entry.filePath = t_sequenceFilePath;
    entry.name = t_name;
    entry.isDefault = makeDefault;

    song->sequences.append(entry);
    emit sequenceAdded(t_songId, entry.id);
    if(makeDefault)
        emit defaultSequenceChanged(t_songId, entry.id);
    return &song->sequences.last();
}

void SongLibrary::removeSequence(qint64 t_songId, qint64 t_sequenceId)
{
    if(!isOpen())
        return;

    SongLibraryEntry *song = findSongById(t_songId);
    if(!song)
        return;

    int index = -1;
    for(int i = 0; i < song->sequences.size(); ++i)
        if(song->sequences[i].id == t_sequenceId) { index = i; break; }
    if(index < 0)
        return;

    const bool wasDefault = song->sequences[index].isDefault;

    QSqlDatabase db = QSqlDatabase::database(m_impl->connectionName);
    QSqlQuery query(db);
    query.prepare("DELETE FROM sequences WHERE id = ?");
    query.addBindValue(t_sequenceId);
    if(!query.exec())
    {
        qWarning() << "SongLibrary: delete sequence failed" << query.lastError().text();
        return;
    }

    song->sequences.removeAt(index);
    emit sequenceRemoved(t_songId, t_sequenceId);

    if(wasDefault && !song->sequences.isEmpty())
        setDefaultSequence(t_songId, song->sequences.first().id);
}

void SongLibrary::setDefaultSequence(qint64 t_songId, qint64 t_sequenceId)
{
    if(!isOpen())
        return;

    SongLibraryEntry *song = findSongById(t_songId);
    if(!song)
        return;

    bool found = false;
    for(const auto &seq : std::as_const(song->sequences))
        if(seq.id == t_sequenceId) { found = true; break; }
    if(!found)
        return;

    QSqlDatabase db = QSqlDatabase::database(m_impl->connectionName);

    // Clear before set, never both at once, so the "one default per song"
    // partial unique index is never violated between these two statements.
    QSqlQuery clearQuery(db);
    clearQuery.prepare("UPDATE sequences SET is_default = 0 WHERE song_id = ?");
    clearQuery.addBindValue(t_songId);
    clearQuery.exec();

    QSqlQuery setQuery(db);
    setQuery.prepare("UPDATE sequences SET is_default = 1 WHERE id = ?");
    setQuery.addBindValue(t_sequenceId);
    if(!setQuery.exec())
    {
        qWarning() << "SongLibrary: set default sequence failed" << setQuery.lastError().text();
        return;
    }

    for(auto &seq : song->sequences)
        seq.isDefault = (seq.id == t_sequenceId);

    emit defaultSequenceChanged(t_songId, t_sequenceId);
}

SongData SongLibrary::songDataFor(const SongLibraryEntry &t_song) const
{
    SongData data;
    if(!m_impl->libraryPath.isEmpty())
        data.load(m_impl->songDataPath(t_song.trackKey));

    if(data.title().isEmpty() && data.artist().isEmpty())
    {
        data.setTitle(t_song.title);
        data.setArtist(t_song.artist);
        data.setDuration(t_song.duration);
        data.setTrackKey(t_song.trackKey);
    }
    return data;
}

bool SongLibrary::saveSongData(const SongLibraryEntry &t_song, const SongData &t_data) const
{
    if(m_impl->libraryPath.isEmpty())
        return false;
    QDir(m_impl->libraryPath).mkpath("songs");
    return t_data.save(m_impl->songDataPath(t_song.trackKey));
}

} // namespace photon

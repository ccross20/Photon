#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include "songlibrarytest.h"
#include "library/songlibrary.h"
#include "audio/songdata.h"

namespace photon {

namespace {

// A .song file the way the library writes them: named by its key.
QByteArray writeSong(const QDir &t_library, const QString &t_title, const QString &t_artist, double t_duration)
{
    SongData data;
    data.setTitle(t_title);
    data.setArtist(t_artist);
    data.setDuration(t_duration);
    data.setSourcePath("netsearch://td1");
    const QByteArray key = data.trackKey();
    t_library.mkpath("songs");
    data.save(t_library.filePath("songs/" + QString::fromLatin1(key) + ".song"));
    return key;
}

void writeSequence(const QDir &t_library, const QString &t_fileName, const QString &t_name,
                   const QByteArray &t_songKey = QByteArray())
{
    QJsonObject json;
    json.insert("name", t_name);
    json.insert("layers", QJsonArray());
    if(!t_songKey.isEmpty())
        json.insert("songTrackKey", QString::fromLatin1(t_songKey));
    t_library.mkpath("sequences");
    QFile file(t_library.filePath("sequences/" + t_fileName));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(json).toJson());
}

} // namespace

SongLibraryTest::SongLibraryTest(QObject *parent) : QObject{parent}
{
}

void SongLibraryTest::initTestCase()
{
    // The SQLite driver is a plugin, which needs an application object. The
    // runner doesn't create one; left alive for the rest of the run.
    if(!QCoreApplication::instance())
    {
        static int argc = 1;
        static char name[] = "photon-core-test";
        static char *argv[] = {name, nullptr};
        new QCoreApplication(argc, argv);
    }
    QVERIFY(QSqlDatabase::isDriverAvailable("QSQLITE"));
}

// Only songs/ and sequences/ copied over: the catalog is rebuilt from them.
void SongLibraryTest::importsSongsAndSequencesFromFolder()
{
    QTemporaryDir temp;
    const QDir library(temp.path());

    const QByteArray swingKey = writeSong(library, "Booty Swing", "Parov Stelar", 198);
    writeSong(library, "Blinding Lights", "The Weeknd", 200);
    // Linked by the key a save records, despite the unrelated name...
    writeSequence(library, "My Swing Show.seq", "My Swing Show", swingKey);
    // ...or, for an older file, by its name matching one title.
    writeSequence(library, "Blinding Lights.seq", "Blinding Lights");
    // No key and no matching title: left unlinked.
    writeSequence(library, "Unknown.seq", "Unknown");

    SongLibrary songs;
    QVERIFY(songs.open(temp.path()));
    QCOMPARE(songs.songCount(), 2);

    const SongLibraryEntry *swing = songs.findSongByTrackKey(swingKey);
    QVERIFY(swing);
    QCOMPARE(swing->title, QString("Booty Swing"));
    QCOMPARE(swing->sequences.size(), 1);
    QCOMPARE(swing->sequences.first().name, QString("My Swing Show"));
    QVERIFY(swing->sequences.first().isDefault);

    const QString lightsPath = library.filePath("sequences/Blinding Lights.seq");
    const SongLibraryEntry *lights = songs.findSongBySequencePath(lightsPath);
    QVERIFY(lights);
    QCOMPARE(lights->title, QString("Blinding Lights"));
    QVERIFY(!songs.findSongBySequencePath(library.filePath("sequences/Unknown.seq")));

    // Opening again adds nothing new.
    songs.close();
    QVERIFY(songs.open(temp.path()));
    QCOMPARE(songs.songCount(), 2);
    QCOMPARE(songs.findSongByTrackKey(swingKey)->sequences.size(), 1);
}

// The whole folder copied, database included, whose rows hold absolute paths
// from the other computer: they resolve here and are stored portably.
void SongLibraryTest::relinksCopiedDatabase()
{
    QTemporaryDir temp;
    const QDir library(temp.path());
    const QByteArray key = writeSong(library, "Booty Swing", "Parov Stelar", 198);
    writeSequence(library, "Booty Swing.seq", "Booty Swing");

    {
        SongLibrary songs;
        QVERIFY(songs.open(temp.path()));
        QCOMPARE(songs.findSongByTrackKey(key)->sequences.size(), 1);
    }

    // Rewrite the row as the other computer would have stored it.
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "songlibrarytest");
        db.setDatabaseName(library.filePath("library.sqlite"));
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("UPDATE sequences SET file_path = '/Users/someone-else/Music/Photon/sequences/Booty Swing.seq'"));
        db.close();
    }
    QSqlDatabase::removeDatabase("songlibrarytest");

    SongLibrary songs;
    QVERIFY(songs.open(temp.path()));
    const SongLibraryEntry *song = songs.findSongByTrackKey(key);
    QVERIFY(song);
    QCOMPARE(song->sequences.size(), 1);   // relinked, not added again
    QCOMPARE(song->sequences.first().filePath,
             QDir::cleanPath(library.filePath("sequences/Booty Swing.seq")));

    // And stored relative to the library from now on.
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "songlibrarytest2");
        db.setDatabaseName(library.filePath("library.sqlite"));
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("SELECT file_path FROM sequences"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QString("sequences/Booty Swing.seq"));
        db.close();
    }
    QSqlDatabase::removeDatabase("songlibrarytest2");
}

} // namespace photon

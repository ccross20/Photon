#include <QCoreApplication>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include "virtualdjplaybacktest.h"
#include "library/songlibrary.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"
#include "virtualdj/virtualdjconnector.h"
#include "virtualdj/virtualdjplayback.h"

namespace photon {

namespace {

// A sequence that's already "loaded" at a given path, so the playback reuses
// it instead of loading the file (which needs a running app).
Sequence *loadedSequence(const QString &t_path, SequenceCollection *t_collection)
{
    auto *sequence = new Sequence;
    LoadContext context{nullptr};
    sequence->readFromJson(QJsonObject{{"name", t_path}, {"filePath", t_path}}, context);
    t_collection->addSequence(sequence);
    return sequence;
}

} // namespace

VirtualDJPlaybackTest::VirtualDJPlaybackTest(QObject *parent) : QObject{parent}
{
}

VirtualDJPlaybackTest::~VirtualDJPlaybackTest() = default;

void VirtualDJPlaybackTest::initTestCase()
{
    // SQLite's driver plugin needs an application instance to load.
    int argc = 0;
    if(!QCoreApplication::instance())
        m_app = std::make_unique<QCoreApplication>(argc, nullptr);

    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_library = new SongLibrary;
    QVERIFY(m_library->open(m_dir->path()));

    auto *songA = m_library->addOrUpdateFromVdj("Title A", "Artist A", 200.0, "/music/a.mp3");
    QVERIFY(songA);
    m_library->addSequence(songA->id, "/sequences/a.seq", "A");

    auto *songB = m_library->addLocalSong("/music/b.mp3", "Title B", "Artist B", 180.4);
    QVERIFY(songB);
    m_library->addSequence(songB->id, "/sequences/b.seq", "B");

    auto *songC = m_library->addOrUpdateFromVdj("Library Name", "Artist C", 150.0, "/music/c.mp3");
    QVERIFY(songC);
    m_library->addSequence(songC->id, "/sequences/c.seq", "C");

    m_sequences = new SequenceCollection(false);
    m_seqA = loadedSequence("/sequences/a.seq", m_sequences);
    m_seqB = loadedSequence("/sequences/b.seq", m_sequences);
    m_seqC = loadedSequence("/sequences/c.seq", m_sequences);

    m_connector = new VirtualDJConnector;
    m_playback = new VirtualDJPlayback(m_connector, m_library, m_sequences);
}

void VirtualDJPlaybackTest::cleanupTestCase()
{
    delete m_playback;
    delete m_connector;
    delete m_sequences;
    delete m_seqA;
    delete m_seqB;
    delete m_seqC;
    delete m_library;
    m_dir.reset();
    m_app.reset();
}

void VirtualDJPlaybackTest::sendTelemetry(const QString &t_title, const QString &t_artist, double t_length,
                                          const QString &t_path, double t_time)
{
    m_connector->title = t_title;
    m_connector->artist = t_artist;
    m_connector->songLength = t_length;
    m_connector->path = t_path;
    m_connector->time = t_time;
    emit m_connector->dataUpdated();
}

Sequence *VirtualDJPlaybackTest::currentSequence(double *t_songTime) const
{
    Sequence *found = nullptr;
    m_playback->process([&](Sequence *sequence, double songTime) {
        found = sequence;
        if(t_songTime)
            *t_songTime = songTime;
    });
    return found;
}

void VirtualDJPlaybackTest::matchesExactTrackKey()
{
    sendTelemetry("Title A", "Artist A", 200.0, "/elsewhere/a.mp3", 12.0);
    double time = 0.0;
    QCOMPARE(currentSequence(&time), m_seqA);
    QVERIFY(qAbs(time - 12.0) < 0.01);
}

void VirtualDJPlaybackTest::matchesArtistTitleWithinTolerance()
{
    // Different case/whitespace and a length 1.1 s off the local analysis.
    sendTelemetry(" title b", "ARTIST B", 181.5, "/stream/b", 3.0);
    QCOMPARE(currentSequence(), m_seqB);
}

void VirtualDJPlaybackTest::matchesSourcePath()
{
    sendTelemetry("Renamed In VDJ", "Artist C", 150.0, "/music/c.mp3", 1.0);
    QCOMPARE(currentSequence(), m_seqC);
}

void VirtualDJPlaybackTest::unmatchedTrackPlaysNothing()
{
    sendTelemetry("Unknown", "Nobody", 99.0, "/music/unknown.mp3", 1.0);
    QCOMPARE(currentSequence(), nullptr);
    QVERIFY(m_playback->status().contains("Not in Song Library"));
}

void VirtualDJPlaybackTest::extrapolatesWhilePlayingAndHoldsWhenPaused()
{
    sendTelemetry("Title A", "Artist A", 200.0, "/elsewhere/a.mp3", 20.0);
    QThread::msleep(60);
    sendTelemetry("Title A", "Artist A", 200.0, "/elsewhere/a.mp3", 20.06);

    // Between ticks the position keeps moving at the playback rate.
    QThread::msleep(100);
    double time = 0.0;
    QCOMPARE(currentSequence(&time), m_seqA);
    QVERIFY2(time > 20.12 && time < 20.3, qPrintable(QString::number(time)));

    // Same position for longer than the pause threshold: hold still.
    QThread::msleep(350);
    sendTelemetry("Title A", "Artist A", 200.0, "/elsewhere/a.mp3", 20.06);
    QThread::msleep(100);
    currentSequence(&time);
    QCOMPARE(time, 20.06);
}

} // namespace photon

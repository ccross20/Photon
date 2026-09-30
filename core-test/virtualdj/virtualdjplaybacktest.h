#ifndef PHOTON_VIRTUALDJPLAYBACKTEST_H
#define PHOTON_VIRTUALDJPLAYBACKTEST_H

#include <QObject>
#include <memory>

class QCoreApplication;
class QTemporaryDir;

namespace photon {

class SongLibrary;
class SequenceCollection;
class Sequence;
class VirtualDJConnector;
class VirtualDJPlayback;

class VirtualDJPlaybackTest : public QObject
{
    Q_OBJECT

public:
    VirtualDJPlaybackTest(QObject *parent = nullptr);
    ~VirtualDJPlaybackTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void matchesExactTrackKey();
    void matchesArtistTitleWithinTolerance();
    void matchesSourcePath();
    void unmatchedTrackPlaysNothing();
    void extrapolatesWhilePlayingAndHoldsWhenPaused();

private:
    void sendTelemetry(const QString &title, const QString &artist, double length,
                       const QString &path, double time);
    Sequence *currentSequence(double *songTime = nullptr) const;

    std::unique_ptr<QCoreApplication> m_app;
    std::unique_ptr<QTemporaryDir> m_dir;
    SongLibrary *m_library = nullptr;
    SequenceCollection *m_sequences = nullptr;
    VirtualDJConnector *m_connector = nullptr;
    VirtualDJPlayback *m_playback = nullptr;
    Sequence *m_seqA = nullptr;
    Sequence *m_seqB = nullptr;
    Sequence *m_seqC = nullptr;
};

} // namespace photon

#endif // PHOTON_VIRTUALDJPLAYBACKTEST_H

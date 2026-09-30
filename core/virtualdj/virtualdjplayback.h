#ifndef PHOTON_VIRTUALDJPLAYBACK_H
#define PHOTON_VIRTUALDJPLAYBACK_H

#include <functional>
#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include "photon-global.h"

namespace photon {

// Follows whatever VirtualDJ is playing: matches the track to a Song Library
// song, loads that song's default sequence, and tracks the song position so
// VirtualDJPlayerNode can play the sequence in step. Lives on the main thread
// (telemetry and sequence loading are main-thread work); the node reads it
// from the eval thread through process().
class PHOTONCORE_EXPORT VirtualDJPlayback : public QObject
{
    Q_OBJECT
public:
    VirtualDJPlayback(VirtualDJConnector *, SongLibrary *, SequenceCollection *, QObject *parent = nullptr);
    ~VirtualDJPlayback() override;

    // Calls fn with the matched sequence and the current song position in
    // seconds (extrapolated between telemetry ticks). Returns false without
    // calling it when nothing matched or VirtualDJ has gone quiet. The
    // sequence can't be unloaded while fn runs. Safe from any thread.
    bool process(const std::function<void(Sequence *, double songTime)> &fn) const;

    // Human-readable match state, e.g. for logging or a status display.
    QString status() const;

signals:
    void statusChanged(const QString &);

private slots:
    void dataUpdated();
    void sequenceWillBeRemoved(photon::Sequence *);

private:
    void matchTrack();
    void setSequence(Sequence *, bool loadedHere);
    void setStatus(const QString &);

    VirtualDJConnector *m_connector;
    SongLibrary *m_library;
    SequenceCollection *m_sequences;

    QString m_trackIdentity;
    QString m_status;
    bool m_loadedHere = false;   // unload it again when the track changes

    // Written on the main thread, read by process() on the eval thread.
    mutable QMutex m_mutex;
    QElapsedTimer m_clock;
    Sequence *m_sequence = nullptr;
    double m_songTime = 0.0;
    qint64 m_receivedMs = 0;
    qint64 m_lastMoveMs = 0;
    double m_rate = 1.0;
    bool m_playing = false;
};

} // namespace photon

#endif // PHOTON_VIRTUALDJPLAYBACK_H

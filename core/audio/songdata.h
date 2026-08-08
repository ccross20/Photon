#ifndef PHOTON_SONGDATA_H
#define PHOTON_SONGDATA_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include "photon-global.h"

class QDataStream;

namespace photon {

/**
 * A named, regularly-sampled continuous signal over song time - a level or
 * frequency-band envelope, etc. Values are stored as raw floats at a fixed sample
 * rate; effects query value(time) with linear interpolation and normalise as they
 * see fit. This is the piece the current audio path lacks: CueLayer already holds
 * beats, but nothing holds a level/spectrum envelope over time.
 */
class PHOTONCORE_EXPORT FeatureTrack
{
public:
    FeatureTrack() = default;
    FeatureTrack(const QByteArray &id, double sampleRate);

    QByteArray id() const { return m_id; }          // stable key, e.g. "level", "low"
    void setId(const QByteArray &id) { m_id = id; }
    QString name() const { return m_name; }          // display label
    void setName(const QString &name) { m_name = name; }

    double sampleRate() const { return m_sampleRate; }   // samples per second
    void setSampleRate(double rate) { m_sampleRate = rate; }
    double startTime() const { return m_startTime; }     // song time of sample 0
    void setStartTime(double t) { m_startTime = t; }

    int sampleCount() const { return m_samples.size(); }
    bool isEmpty() const { return m_samples.isEmpty(); }
    double duration() const;                              // span covered, in seconds

    const QVector<float> &samples() const { return m_samples; }
    void setSamples(const QVector<float> &samples) { m_samples = samples; }
    void reserve(int count) { m_samples.reserve(count); }
    void append(float value) { m_samples.append(value); }   // for incremental recording
    void clear() { m_samples.clear(); }

    // Interpolated value at a song time (seconds). Clamps at the ends; returns 0
    // when the track is empty.
    float value(double time) const;

    // Min/max across all samples, for normalisation and display. Returns false and
    // leaves the outputs untouched when empty.
    bool range(float &outMin, float &outMax) const;

    // Min/max across the raw samples falling within [startTime, endTime) - the
    // per-pixel-column query a waveform-style viewer needs so it can draw an
    // envelope band at screen resolution instead of a naive polyline through every
    // sample. Returns false and leaves the outputs untouched when the range covers
    // no samples (e.g. it falls entirely outside the track).
    bool rangeBetween(double startTime, double endTime, float &outMin, float &outMax) const;

    void write(QDataStream &) const;
    void read(QDataStream &);

private:
    QByteArray m_id;
    QString m_name;
    double m_sampleRate = 0.0;
    double m_startTime = 0.0;
    QVector<float> m_samples;
};


/**
 * The song's beat grid: absolute beat times (seconds) plus a nominal bpm and bar
 * length. Provides the phase/bar-phase/nearest-beat queries the VirtualDJConnector
 * currently derives live, but from stored beats so effects can be authored and
 * scrubbed against a timeline. Beats are kept sorted ascending.
 *
 * Bar alignment: by default beats[0] is assumed to be a downbeat (beat-in-bar 0).
 * When a producer actually knows each beat's position in the bar (e.g. a bar/beat
 * tracker, as opposed to a plain onset detector), it should call
 * setBarStartOffset() with beats[0]'s true 0-based position so barPhase() lines up
 * with the real downbeats instead of guessing from the first detected beat.
 */
class PHOTONCORE_EXPORT BeatGrid
{
public:
    const QVector<double> &beats() const { return m_beats; }
    void setBeats(const QVector<double> &beats);   // stores sorted
    void addBeat(double time);                     // keeps order
    void clear() { m_beats.clear(); }
    bool isEmpty() const { return m_beats.isEmpty(); }
    int count() const { return m_beats.size(); }

    double bpm() const { return m_bpm; }
    void setBpm(double bpm) { m_bpm = bpm; }
    int beatsPerBar() const { return m_beatsPerBar; }
    void setBeatsPerBar(int n) { m_beatsPerBar = n; }

    // 0-based beat-in-bar position of beats[0]; see class comment. Default 0.
    int barStartOffset() const { return m_barStartOffset; }
    void setBarStartOffset(int offset) { m_barStartOffset = offset; }

    // Index of the last beat at or before `time`, or -1 if before the first beat.
    int beatIndexBefore(double time) const;
    // Beat count since the first beat (== beatIndexBefore); convenient for modulo.
    int beatNumber(double time) const { return beatIndexBefore(time); }

    // Phase 0..1 through the current beat interval; 0 outside the grid.
    double phase(double time) const;
    // Phase 0..1 through the current bar (beatsPerBar beats).
    double barPhase(double time) const;

    // Nearest beat within `tolerance` seconds. Returns false if none.
    bool nearestBeat(double time, double *outTime, double tolerance = 0.1) const;

    void write(QDataStream &) const;
    void read(QDataStream &);

private:
    // Interval length around beat index i, falling back to bpm at the edges.
    double intervalAt(int index) const;

    QVector<double> m_beats;
    double m_bpm = 0.0;
    int m_beatsPerBar = 4;
    int m_barStartOffset = 0;
};


/**
 * Source-agnostic analysis of one track - the companion-file contents. Bundles
 * track identity + metadata, a BeatGrid, and any number of FeatureTracks (level,
 * low/mid/high, ...). Produced by an offline analyser (Vamp), a recording of the
 * VirtualDJ stream, or the live stream; consumed by effects/graph nodes via the
 * query API. Self-contained so it can be stored in a library keyed by trackKey()
 * and used in place of the decoded audio file.
 */
class PHOTONCORE_EXPORT SongData
{
public:
    // Standard feature ids. Producers may add others.
    static const QByteArray FeatureLevel;   // "level" - overall loudness
    static const QByteArray FeatureLow;     // "low"   - low band energy (local-file analysis)
    static const QByteArray FeatureMid;     // "mid"
    static const QByteArray FeatureHigh;    // "high"
    // Stem levels (VirtualDJ live capture - real-time AI stem separation, an
    // alternative to the frequency bands above for VDJ-sourced tracks).
    static const QByteArray FeatureVocal;
    static const QByteArray FeatureInstru;
    static const QByteArray FeatureBass;
    static const QByteArray FeatureKick;
    static const QByteArray FeatureHiHat;

    // --- identity / metadata ---
    QString title() const { return m_title; }
    void setTitle(const QString &t) { m_title = t; }
    QString artist() const { return m_artist; }
    void setArtist(const QString &a) { m_artist = a; }
    QString sourcePath() const { return m_sourcePath; }   // local file, empty when streamed
    void setSourcePath(const QString &p) { m_sourcePath = p; }
    QByteArray source() const { return m_source; }         // "vamp", "virtualdj-record", ...
    void setSource(const QByteArray &s) { m_source = s; }
    double duration() const { return m_duration; }         // seconds
    void setDuration(double d) { m_duration = d; }
    double bpm() const { return m_bpm; }
    void setBpm(double b) { m_bpm = b; }

    // True when there's nothing worth persisting (no beats, features, or metadata).
    bool isEmpty() const;

    // Stable identity used to match a track to its stored analysis. Explicitly set
    // when known (e.g. a VirtualDJ id); otherwise derive one from the metadata.
    QByteArray trackKey() const;
    void setTrackKey(const QByteArray &key) { m_trackKey = key; }
    static QByteArray makeTrackKey(const QString &artist, const QString &title, double duration);

    // --- beats ---
    BeatGrid &beats() { return m_beats; }
    const BeatGrid &beats() const { return m_beats; }

    // --- feature tracks ---
    const QVector<FeatureTrack> &features() const { return m_features; }
    FeatureTrack *feature(const QByteArray &id);              // nullptr if absent
    const FeatureTrack *feature(const QByteArray &id) const;
    FeatureTrack &addFeature(const QByteArray &id, double sampleRate);   // creates/replaces
    bool hasFeature(const QByteArray &id) const { return feature(id) != nullptr; }

    // Convenience: interpolated feature value at a song time, or 0 if the feature
    // isn't present.
    float featureValue(const QByteArray &id, double time) const;

    // --- serialization ---
    // Binary sidecar file (magic + version + QDataStream payload). The bulk is
    // float/double envelope and beat arrays, so a byte format is far more compact
    // than JSON.
    void write(QDataStream &) const;
    void read(QDataStream &);
    bool save(const QString &path) const;
    bool load(const QString &path);

private:
    QString m_title;
    QString m_artist;
    QString m_sourcePath;
    QByteArray m_source;
    QByteArray m_trackKey;
    double m_duration = 0.0;
    double m_bpm = 0.0;
    BeatGrid m_beats;
    QVector<FeatureTrack> m_features;
};

} // namespace photon

#endif // PHOTON_SONGDATA_H

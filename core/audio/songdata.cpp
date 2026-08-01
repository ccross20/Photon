#include <QDataStream>
#include <QCryptographicHash>
#include <QFile>
#include <algorithm>
#include <cmath>
#include <limits>
#include "songdata.h"

namespace {
    // Sidecar file header. 'PSNG' = Photon SoNG analysis.
    constexpr quint32 kMagic = 0x50534E47;
    constexpr quint32 kVersion = 1;
    // Pinned so files stay readable across Qt releases.
    constexpr int kStreamVersion = QDataStream::Qt_6_0;
}

namespace photon {

// ---------------------------------------------------------------------------
// FeatureTrack
// ---------------------------------------------------------------------------

FeatureTrack::FeatureTrack(const QByteArray &t_id, double t_sampleRate)
    : m_id(t_id), m_sampleRate(t_sampleRate)
{
}

double FeatureTrack::duration() const
{
    if(m_samples.isEmpty() || m_sampleRate <= 0.0)
        return 0.0;
    return m_samples.size() / m_sampleRate;
}

float FeatureTrack::value(double t_time) const
{
    if(m_samples.isEmpty())
        return 0.0f;
    if(m_sampleRate <= 0.0)
        return m_samples.first();

    const double pos = (t_time - m_startTime) * m_sampleRate;
    if(pos <= 0.0)
        return m_samples.first();
    if(pos >= m_samples.size() - 1)
        return m_samples.last();

    const int i = static_cast<int>(std::floor(pos));
    const float frac = static_cast<float>(pos - i);
    return m_samples[i] + (m_samples[i + 1] - m_samples[i]) * frac;
}

bool FeatureTrack::range(float &t_outMin, float &t_outMax) const
{
    if(m_samples.isEmpty())
        return false;

    float lo = m_samples.first();
    float hi = m_samples.first();
    for(float v : m_samples)
    {
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    t_outMin = lo;
    t_outMax = hi;
    return true;
}

bool FeatureTrack::rangeBetween(double t_startTime, double t_endTime, float &t_outMin, float &t_outMax) const
{
    if(m_samples.isEmpty() || m_sampleRate <= 0.0)
        return false;

    // Raw sample indices covering [startTime, endTime), clamped to the track.
    const int lastIndex = static_cast<int>(m_samples.size()) - 1;
    int i0 = static_cast<int>(std::floor((t_startTime - m_startTime) * m_sampleRate));
    int i1 = static_cast<int>(std::ceil((t_endTime - m_startTime) * m_sampleRate));
    i0 = std::clamp(i0, 0, lastIndex);
    i1 = std::clamp(i1, 0, lastIndex);
    if(i1 < i0)
        return false;

    float lo = m_samples[i0];
    float hi = m_samples[i0];
    for(int i = i0; i <= i1; ++i)
    {
        lo = std::min(lo, m_samples[i]);
        hi = std::max(hi, m_samples[i]);
    }
    t_outMin = lo;
    t_outMax = hi;
    return true;
}

void FeatureTrack::write(QDataStream &t_ds) const
{
    t_ds << m_id << m_name << m_sampleRate << m_startTime;
    t_ds << quint32(m_samples.size());
    // Raw sample blob (native-endian float32). Photon's targets are all
    // little-endian; revisit if that ever stops being true.
    if(!m_samples.isEmpty())
        t_ds.writeRawData(reinterpret_cast<const char *>(m_samples.constData()),
                          m_samples.size() * static_cast<int>(sizeof(float)));
}

void FeatureTrack::read(QDataStream &t_ds)
{
    t_ds >> m_id >> m_name >> m_sampleRate >> m_startTime;
    quint32 count = 0;
    t_ds >> count;
    m_samples.resize(count);
    if(count > 0)
        t_ds.readRawData(reinterpret_cast<char *>(m_samples.data()),
                         count * static_cast<int>(sizeof(float)));
}


// ---------------------------------------------------------------------------
// BeatGrid
// ---------------------------------------------------------------------------

void BeatGrid::setBeats(const QVector<double> &t_beats)
{
    m_beats = t_beats;
    std::sort(m_beats.begin(), m_beats.end());
}

void BeatGrid::addBeat(double t_time)
{
    // Keep sorted by inserting at the right spot.
    const auto it = std::lower_bound(m_beats.begin(), m_beats.end(), t_time);
    m_beats.insert(it, t_time);
}

int BeatGrid::beatIndexBefore(double t_time) const
{
    if(m_beats.isEmpty() || t_time < m_beats.first())
        return -1;
    // Last beat <= time.
    const auto it = std::upper_bound(m_beats.begin(), m_beats.end(), t_time);
    return static_cast<int>(it - m_beats.begin()) - 1;
}

double BeatGrid::intervalAt(int t_index) const
{
    // Prefer the measured interval between adjacent beats; fall back to the nominal
    // bpm at the grid edges (or when only one beat exists).
    if(t_index >= 0 && t_index + 1 < m_beats.size())
        return m_beats[t_index + 1] - m_beats[t_index];
    if(t_index - 1 >= 0 && t_index < m_beats.size())
        return m_beats[t_index] - m_beats[t_index - 1];
    if(m_bpm > 0.0)
        return 60.0 / m_bpm;
    return 0.0;
}

double BeatGrid::phase(double t_time) const
{
    const int i = beatIndexBefore(t_time);
    if(i < 0)
        return 0.0;

    const double interval = intervalAt(i);
    if(interval <= 0.0)
        return 0.0;

    const double p = (t_time - m_beats[i]) / interval;
    return std::clamp(p, 0.0, 1.0);
}

double BeatGrid::barPhase(double t_time) const
{
    const int i = beatIndexBefore(t_time);
    if(i < 0 || m_beatsPerBar <= 0)
        return 0.0;

    const int beatInBar = (i + m_barStartOffset) % m_beatsPerBar;
    return (beatInBar + phase(t_time)) / m_beatsPerBar;
}

bool BeatGrid::nearestBeat(double t_time, double *t_outTime, double t_tolerance) const
{
    if(m_beats.isEmpty())
        return false;

    const auto it = std::lower_bound(m_beats.begin(), m_beats.end(), t_time);

    double best = 0.0;
    double bestDist = std::numeric_limits<double>::max();

    // Check the beat at/after the time and the one before it.
    if(it != m_beats.end())
    {
        const double d = std::abs(*it - t_time);
        if(d < bestDist) { bestDist = d; best = *it; }
    }
    if(it != m_beats.begin())
    {
        const double prev = *(it - 1);
        const double d = std::abs(prev - t_time);
        if(d < bestDist) { bestDist = d; best = prev; }
    }

    if(bestDist > t_tolerance)
        return false;

    if(t_outTime)
        *t_outTime = best;
    return true;
}

void BeatGrid::write(QDataStream &t_ds) const
{
    t_ds << m_bpm << qint32(m_beatsPerBar) << qint32(m_barStartOffset);
    t_ds << m_beats;   // portable QVector<double>
}

void BeatGrid::read(QDataStream &t_ds)
{
    qint32 beatsPerBar = 4;
    qint32 barStartOffset = 0;
    t_ds >> m_bpm >> beatsPerBar >> barStartOffset;
    m_beatsPerBar = beatsPerBar;
    m_barStartOffset = barStartOffset;
    t_ds >> m_beats;
    std::sort(m_beats.begin(), m_beats.end());
}


// ---------------------------------------------------------------------------
// SongData
// ---------------------------------------------------------------------------

const QByteArray SongData::FeatureLevel  = "level";
const QByteArray SongData::FeatureLow    = "low";
const QByteArray SongData::FeatureMid    = "mid";
const QByteArray SongData::FeatureHigh   = "high";
const QByteArray SongData::FeatureVocal  = "vocal";
const QByteArray SongData::FeatureInstru = "instru";
const QByteArray SongData::FeatureBass   = "bass";
const QByteArray SongData::FeatureKick   = "kick";
const QByteArray SongData::FeatureHiHat  = "hihat";

QByteArray SongData::makeTrackKey(const QString &t_artist, const QString &t_title, double t_duration)
{
    // Stable-ish identity for matching a playing track to stored analysis when no
    // explicit id is available (e.g. streamed tracks with no file path). Rounded
    // duration guards against tiny reported-length jitter.
    const QString basis = t_artist.trimmed().toLower() + "|" +
                          t_title.trimmed().toLower() + "|" +
                          QString::number(std::llround(t_duration));
    return QCryptographicHash::hash(basis.toUtf8(), QCryptographicHash::Sha1).toHex();
}

bool SongData::isEmpty() const
{
    return m_beats.isEmpty() && m_features.isEmpty() &&
           m_title.isEmpty() && m_artist.isEmpty() &&
           m_trackKey.isEmpty() && m_duration == 0.0;
}

QByteArray SongData::trackKey() const
{
    if(!m_trackKey.isEmpty())
        return m_trackKey;
    return makeTrackKey(m_artist, m_title, m_duration);
}

FeatureTrack *SongData::feature(const QByteArray &t_id)
{
    for(FeatureTrack &f : m_features)
        if(f.id() == t_id)
            return &f;
    return nullptr;
}

const FeatureTrack *SongData::feature(const QByteArray &t_id) const
{
    for(const FeatureTrack &f : m_features)
        if(f.id() == t_id)
            return &f;
    return nullptr;
}

FeatureTrack &SongData::addFeature(const QByteArray &t_id, double t_sampleRate)
{
    if(FeatureTrack *existing = feature(t_id))
    {
        *existing = FeatureTrack(t_id, t_sampleRate);
        return *existing;
    }
    m_features.append(FeatureTrack(t_id, t_sampleRate));
    return m_features.last();
}

float SongData::featureValue(const QByteArray &t_id, double t_time) const
{
    const FeatureTrack *f = feature(t_id);
    return f ? f->value(t_time) : 0.0f;
}

void SongData::write(QDataStream &t_ds) const
{
    // Store the explicit key (may be empty); trackKey() still derives one on demand.
    t_ds << m_title << m_artist << m_sourcePath << m_source << m_trackKey
         << m_duration << m_bpm;

    m_beats.write(t_ds);

    t_ds << quint32(m_features.size());
    for(const FeatureTrack &f : m_features)
        f.write(t_ds);
}

void SongData::read(QDataStream &t_ds)
{
    t_ds >> m_title >> m_artist >> m_sourcePath >> m_source >> m_trackKey
         >> m_duration >> m_bpm;

    m_beats.read(t_ds);

    quint32 count = 0;
    t_ds >> count;
    m_features.clear();
    m_features.reserve(count);
    for(quint32 i = 0; i < count; ++i)
    {
        FeatureTrack track;
        track.read(t_ds);
        m_features.append(track);
    }
}

bool SongData::save(const QString &t_path) const
{
    QFile file(t_path);
    if(!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream ds(&file);
    ds.setVersion(kStreamVersion);
    ds << kMagic << kVersion;
    write(ds);
    return ds.status() == QDataStream::Ok;
}

bool SongData::load(const QString &t_path)
{
    QFile file(t_path);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream ds(&file);
    ds.setVersion(kStreamVersion);

    quint32 magic = 0;
    quint32 version = 0;
    ds >> magic >> version;
    if(magic != kMagic || version > kVersion)
        return false;   // not ours, or newer than we understand

    read(ds);
    return ds.status() == QDataStream::Ok;
}

} // namespace photon

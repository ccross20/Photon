#include "clockcontroller.h"
#include <QElapsedTimer>
#include <QMutex>
#include <QVector>
#include <QtMath>

namespace keira {

static constexpr int    TapWindowSize           = 4;
static constexpr double TapResetThresholdSecs   = 3.0;
static constexpr double BpmMin                  = 20.0;
static constexpr double BpmMax                  = 300.0;

class ClockController::Impl
{
public:
    QMutex mutex;
    FrameTime time;
    double beatAccumulator = 0.0;  // monotonically increasing beat count (fractional)

    QVector<qint64> tapTimestamps; // ms, capped at TapWindowSize + 1 entries
    QElapsedTimer   tapTimer;
};

ClockController::ClockController()
    : m_impl(new Impl)
{
    m_impl->tapTimer.start();
}

ClockController::~ClockController()
{
    delete m_impl;
}

void ClockController::tick(double deltaSeconds)
{
    QMutexLocker lock(&m_impl->mutex);
    auto &t = m_impl->time;
    t.elapsed += deltaSeconds;
    t.delta    = deltaSeconds;
    t.frame++;

    const double beatsPerSecond = t.bpm / 60.0;
    m_impl->beatAccumulator += beatsPerSecond * deltaSeconds;

    t.beat      = static_cast<int>(m_impl->beatAccumulator);
    t.beatPhase = m_impl->beatAccumulator - t.beat;
    t.bar       = t.beat / t.beatsPerBar;

    const double beatWithinBar = m_impl->beatAccumulator - (t.bar * t.beatsPerBar);
    t.barPhase  = beatWithinBar / t.beatsPerBar;
}

void ClockController::setBpm(double bpm)
{
    QMutexLocker lock(&m_impl->mutex);
    m_impl->time.bpm = qBound(BpmMin, bpm, BpmMax);
}

double ClockController::bpm() const
{
    QMutexLocker lock(&m_impl->mutex);
    return m_impl->time.bpm;
}

void ClockController::setBeatsPerBar(int beatsPerBar)
{
    QMutexLocker lock(&m_impl->mutex);
    m_impl->time.beatsPerBar = qMax(1, beatsPerBar);
}

int ClockController::beatsPerBar() const
{
    QMutexLocker lock(&m_impl->mutex);
    return m_impl->time.beatsPerBar;
}

void ClockController::tap()
{
    QMutexLocker lock(&m_impl->mutex);
    const qint64 now = m_impl->tapTimer.elapsed();
    auto &taps = m_impl->tapTimestamps;

    if (!taps.isEmpty())
    {
        const double secondsSinceLast = (now - taps.last()) / 1000.0;
        if (secondsSinceLast > TapResetThresholdSecs)
            taps.clear();
    }

    taps.append(now);
    if (taps.size() > TapWindowSize + 1)
        taps.removeFirst();

    if (taps.size() >= 2)
    {
        double total = 0.0;
        for (int i = 1; i < taps.size(); ++i)
            total += (taps[i] - taps[i - 1]) / 1000.0;
        // setBpm acquires the mutex too, so set directly here
        m_impl->time.bpm = qBound(BpmMin, 60.0 / (total / (taps.size() - 1)), BpmMax);
    }
}

void ClockController::reset()
{
    QMutexLocker lock(&m_impl->mutex);
    const double bpm      = m_impl->time.bpm;
    const int beatsPerBar = m_impl->time.beatsPerBar;

    m_impl->time          = FrameTime{};
    m_impl->beatAccumulator = 0.0;
    m_impl->tapTimestamps.clear();
    m_impl->time.bpm        = bpm;
    m_impl->time.beatsPerBar = beatsPerBar;
    m_impl->tapTimer.start();
}

FrameTime ClockController::currentTime() const
{
    QMutexLocker lock(&m_impl->mutex);
    return m_impl->time;
}

} // namespace keira

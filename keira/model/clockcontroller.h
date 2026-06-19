#ifndef KEIRA_CLOCKCONTROLLER_H
#define KEIRA_CLOCKCONTROLLER_H

#include "keira-global.h"
#include "model/frametime.h"

namespace keira {

class KEIRA_EXPORT ClockController
{
public:
    ClockController();
    ~ClockController();

    // Advance the clock by one frame. Called by the eval thread each tick.
    void tick(double deltaSeconds);

    void setBpm(double bpm);
    double bpm() const;

    void setBeatsPerBar(int beatsPerBar);
    int beatsPerBar() const;

    // Register a tap for tap-tempo detection.
    void tap();

    void reset();

    FrameTime currentTime() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_CLOCKCONTROLLER_H

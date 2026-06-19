#ifndef KEIRA_FRAMETIME_H
#define KEIRA_FRAMETIME_H

#include <QtGlobal>

namespace keira {

struct FrameTime
{
    double elapsed = 0.0;     // seconds since clock start
    double delta = 0.0;       // seconds since last frame
    double bpm = 120.0;
    double beatPhase = 0.0;   // 0.0–1.0 within current beat
    double barPhase = 0.0;    // 0.0–1.0 within current bar
    int beat = 0;             // total beat count since start
    int bar = 0;              // total bar count since start
    int beatsPerBar = 4;
    quint64 frame = 0;        // total frame count since start
};

} // namespace keira

#endif // KEIRA_FRAMETIME_H

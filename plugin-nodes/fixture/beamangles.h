#ifndef PHOTON_BEAMANGLES_H
#define PHOTON_BEAMANGLES_H

#include <QtMath>
#include <QVector3D>
#include <cmath>

// Shared pan/tilt <-> beam-direction math for the fixture-aiming nodes
// (Look At Target, Look In Direction, Mirror Pan/Tilt). Kept in one place so the
// singularity handling can't drift between them.
namespace photon::beamangles {

// Fixture-local unit direction the beam points for a centred pan/tilt (degrees).
// Matches the visualiser: the beam rests along local -Y, pan rotates it about
// +Y, then tilt about +X, giving
//   ( -sin(pan)*sin(tilt), -cos(tilt), -cos(pan)*sin(tilt) ).
inline QVector3D directionFromPanTilt(double t_panDeg, double t_tiltDeg)
{
    const double pan  = qDegreesToRadians(t_panDeg);
    const double tilt = qDegreesToRadians(t_tiltDeg);
    return QVector3D(float(-std::sin(pan) * std::sin(tilt)),
                     float(-std::cos(tilt)),
                     float(-std::cos(pan) * std::sin(tilt)));
}

// Inverse of directionFromPanTilt: the centred pan/tilt (degrees) that aims the
// beam along t_dir (fixture-local, need not be normalised). Angles are centred
// (0,0 = straight down local -Y), matching the Set Fixture Pan/Tilt nodes.
//
// The same aim is given by both (pan, tilt) and (pan+180, -tilt); near the pan
// axis (target almost straight down/up) pan is ill-defined and the naive
// solution flips 180°, spinning the head. t_prevPan is the caller's previous pan
// output brought back into NON-inverted space - the solution closest to it is
// chosen so the head tilts smoothly through instead of flipping, and pan is held
// at the exact singularity. t_invertPan / t_invertTilt negate the matching
// output as the final step.
inline void panTiltFromDirection(const QVector3D &t_dir, double t_prevPan,
                                 bool t_invertPan, bool t_invertTilt,
                                 double &t_outPan, double &t_outTilt)
{
    QVector3D dir = t_dir;
    if(dir.lengthSquared() < 1e-8f)
    {
        t_outPan = 0.0;
        t_outTilt = 0.0;
        return;
    }
    dir.normalize();

    const double tiltA = qRadiansToDegrees(std::acos(qBound(-1.0f, -dir.y(), 1.0f)));
    const double horiz = std::sqrt(double(dir.x()) * dir.x() + double(dir.z()) * dir.z());

    double pan, tilt;
    if(horiz < 1e-4)
    {
        pan  = t_prevPan;                                // pan undefined here
        tilt = (dir.y() <= 0.0f) ? 0.0 : 180.0;          // straight down / up
    }
    else
    {
        double panA = qRadiansToDegrees(std::atan2(-dir.x(), -dir.z()));
        double panB = panA + 180.0;
        if(panB > 180.0) panB -= 360.0;

        auto panDist = [](double a, double b) {
            return std::abs(std::fmod(a - b + 540.0, 360.0) - 180.0);
        };
        if(panDist(panB, t_prevPan) < panDist(panA, t_prevPan))
        { pan = panB; tilt = -tiltA; }                   // flipped solution, same aim
        else
        { pan = panA; tilt = tiltA; }
    }

    t_outPan  = t_invertPan  ? -pan  : pan;
    t_outTilt = t_invertTilt ? -tilt : tilt;
}

} // namespace photon::beamangles

#endif // PHOTON_BEAMANGLES_H

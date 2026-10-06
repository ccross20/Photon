#ifndef PHOTON_SCENEFALLOFF_H
#define PHOTON_SCENEFALLOFF_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

// A spatial gradient used to spread time across fixtures: each position in the
// rig gets an amount from 0 to 1, which nodes (e.g. Spatial Falloff) turn into
// a per-fixture delay. Laid out in the object's local XY plane - height off
// that plane (local Z) never matters, so on its default flat orientation high
// and low trusses at the same spot get the same amount.
//
//  - Linear:  0 at the origin rising to 1 at `length` along local +Y.
//  - Radial:  0 at the origin rising to 1 at a radius of `length`.
//  - Conical: a radar sweep - 0 along local +Y, rising toward +X, reaching 1
//             after `sweep` degrees.
//
// Past 1 the gradient holds, repeats, or ping-pongs. Mirroring reflects the
// pattern across the local X and/or Y axis before it's measured.
class PHOTONCORE_EXPORT SceneFalloff : public SceneHelperObject
{
    Q_OBJECT
public:
    // Stored by value in project files - append only.
    enum Shape { ShapeLinear = 0, ShapeRadial = 1, ShapeConical = 2 };
    enum Wrap { WrapHold = 0, WrapRepeat = 1, WrapPingPong = 2 };

    SceneFalloff();
    ~SceneFalloff();

    Shape shape() const;
    void setShape(Shape);
    // Linear distance / radial radius over which the amount goes 0 to 1
    // (conical: just the size it's drawn at).
    float length() const;
    void setLength(float);
    // Conical only: degrees of sweep over which the amount goes 0 to 1.
    float sweep() const;
    void setSweep(float);
    Wrap wrap() const;
    void setWrap(Wrap);
    // Reflects across the local X axis (y -> |y|) / local Y axis (x -> |x|).
    bool mirrorAcrossX() const;
    void setMirrorAcrossX(bool);
    bool mirrorAcrossY() const;
    void setMirrorAcrossY(bool);

    // The amount (0..1) at a point in the falloff's own local space.
    double amountAtLocal(const QVector3D &local) const;
    // Before hold/repeat/ping-pong: 0 at the start, 1 at the end, unbounded.
    double rawValueAtLocal(const QVector3D &local) const;
    // The same for a world-space position (e.g. a fixture's).
    double amountAt(const QVector3D &world) const;
    double rawValueAt(const QVector3D &world) const;

    // Hold/repeat/ping-pong applied to a raw value.
    static double wrapped(double raw, Wrap);

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEFALLOFF_H

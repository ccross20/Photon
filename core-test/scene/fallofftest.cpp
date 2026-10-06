#include <QJsonObject>
#include <QMatrix4x4>
#include <QTest>
#include "fallofftest.h"
#include "scene/scenefalloff.h"

namespace photon {

namespace {
bool near(double a, double b) { return std::abs(a - b) < 1e-6; }
}

void FalloffTest::linear()
{
    SceneFalloff falloff;
    falloff.setLength(2.0f);
    QVERIFY(near(falloff.amountAtLocal({0, 0, 0}), 0.0));
    QVERIFY(near(falloff.amountAtLocal({5, 1, 0}), 0.5));   // across the strip doesn't matter
    QVERIFY(near(falloff.amountAtLocal({0, 2, 0}), 1.0));
    QVERIFY(near(falloff.amountAtLocal({0, 7, 0}), 1.0));   // hold past the end
    QVERIFY(near(falloff.amountAtLocal({0, -3, 0}), 0.0));  // and before the start
}

void FalloffTest::wrapModes()
{
    QVERIFY(near(SceneFalloff::wrapped(1.25, SceneFalloff::WrapHold), 1.0));
    QVERIFY(near(SceneFalloff::wrapped(1.25, SceneFalloff::WrapRepeat), 0.25));
    QVERIFY(near(SceneFalloff::wrapped(1.25, SceneFalloff::WrapPingPong), 0.75));
    QVERIFY(near(SceneFalloff::wrapped(2.25, SceneFalloff::WrapPingPong), 0.25));
    QVERIFY(near(SceneFalloff::wrapped(-0.25, SceneFalloff::WrapRepeat), 0.75));
    QVERIFY(near(SceneFalloff::wrapped(-0.25, SceneFalloff::WrapPingPong), 0.25));

    SceneFalloff falloff;
    falloff.setLength(2.0f);
    falloff.setWrap(SceneFalloff::WrapRepeat);
    QVERIFY(near(falloff.amountAtLocal({0, 3, 0}), 0.5));
}

void FalloffTest::mirroring()
{
    SceneFalloff falloff;
    falloff.setLength(2.0f);
    QVERIFY(near(falloff.amountAtLocal({0, -1, 0}), 0.0));
    falloff.setMirrorAcrossX(true);    // centre-out "V"
    QVERIFY(near(falloff.amountAtLocal({0, -1, 0}), 0.5));

    SceneFalloff sweep;
    sweep.setShape(SceneFalloff::ShapeConical);
    const double left = sweep.amountAtLocal({-1, 0, 0});
    QVERIFY(near(left, 0.75));
    sweep.setMirrorAcrossY(true);      // both sides sweep the same way
    QVERIFY(near(sweep.amountAtLocal({-1, 0, 0}), sweep.amountAtLocal({1, 0, 0})));
}

void FalloffTest::radialIgnoresHeight()
{
    SceneFalloff falloff;
    falloff.setShape(SceneFalloff::ShapeRadial);
    falloff.setLength(10.0f);
    QVERIFY(near(falloff.amountAtLocal({3, 4, 0}), 0.5));
    QVERIFY(near(falloff.amountAtLocal({3, 4, 100}), 0.5));   // flat: local Z ignored
    QVERIFY(near(falloff.amountAtLocal({30, 40, 0}), 1.0));
}

void FalloffTest::conicalSweep()
{
    SceneFalloff falloff;
    falloff.setShape(SceneFalloff::ShapeConical);
    // Clockwise from +Y toward +X.
    QVERIFY(near(falloff.amountAtLocal({0, 1, 0}), 0.0));
    QVERIFY(near(falloff.amountAtLocal({1, 0, 0}), 0.25));
    QVERIFY(near(falloff.amountAtLocal({0, -1, 0}), 0.5));
    QVERIFY(near(falloff.amountAtLocal({-1, 0, 0}), 0.75));

    falloff.setSweep(180.0f);
    QVERIFY(near(falloff.amountAtLocal({1, 0, 0}), 0.5));
    QVERIFY(near(falloff.amountAtLocal({-1, 0, 0}), 1.0));    // held past the sweep
}

void FalloffTest::worldTransform()
{
    // Lying flat (as created from the project panel) and moved.
    SceneFalloff falloff;
    falloff.setShape(SceneFalloff::ShapeRadial);
    falloff.setLength(2.0f);
    falloff.setPosition({10, 0, 0});
    falloff.setRotation({-90, 0, 0});

    const QVector3D onFloor = falloff.globalMatrix().map(QVector3D(0, 1, 0));
    QVERIFY(near(falloff.amountAt(onFloor), 0.5));
    // Straight up from that spot is the same distance in plan view.
    QVERIFY(near(falloff.amountAt(onFloor + QVector3D(0, 5, 0)), 0.5));
}

void FalloffTest::loadsOldLinearFalloff()
{
    SceneFalloff falloff;
    LoadContext context{nullptr};
    falloff.readFromJson(QJsonObject{{"typeId", "linearfalloff"}, {"name", "Old"}, {"length", 3.0}}, context);
    QCOMPARE(falloff.typeId(), QByteArray("falloff"));
    QCOMPARE(falloff.shape(), SceneFalloff::ShapeLinear);
    QCOMPARE(falloff.length(), 3.0f);
    QCOMPARE(falloff.wrap(), SceneFalloff::WrapHold);
}

} // namespace photon

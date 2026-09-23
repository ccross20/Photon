#include <QColor>
#include <QTest>
#include "colorcalibrationtest.h"
#include "fixture/capability/fixturecolorcalibration.h"

namespace photon {

namespace {

ColorCalibrationEntry makeEntry(const QString &name, double hue, CapabilityType type, double percent)
{
    ColorCalibrationEntry entry;
    entry.name = name;
    entry.hueDegrees = hue;
    entry.channelPercents.insert(type, percent);
    return entry;
}

}

ColorCalibrationTest::ColorCalibrationTest(QObject *parent)
    : QObject{parent}
{

}

void ColorCalibrationTest::unitTest()
{
    FixtureColorCalibration calibration;
    calibration.hues.append(makeEntry("Red", 0.0, Capability_Red, 1.0));
    calibration.hues.append(makeEntry("Green", 120.0, Capability_Green, 1.0));
    calibration.hues.append(makeEntry("Blue", 240.0, Capability_Blue, 1.0));

    QVERIFY(calibration.isValid());

    // Exactly on an anchor: full red, nothing else.
    {
        const auto percents = resolveCalibratedPercents(calibration, QColor::fromHsv(0, 255, 255));
        QCOMPARE(percents.value(Capability_Red), 1.0);
        QCOMPARE(percents.value(Capability_Green, 0.0), 0.0);
        QCOMPARE(percents.value(Capability_Blue, 0.0), 0.0);
    }

    // Halfway between Red and Green anchors.
    {
        const auto percents = resolveCalibratedPercents(calibration, QColor::fromHsv(60, 255, 255));
        QVERIFY(qAbs(percents.value(Capability_Red) - 0.5) < 0.01);
        QVERIFY(qAbs(percents.value(Capability_Green) - 0.5) < 0.01);
    }

    // Wraps around the ring: halfway between the Blue anchor (240) and the
    // Red anchor (360/0).
    {
        const auto percents = resolveCalibratedPercents(calibration, QColor::fromHsv(300, 255, 255));
        QVERIFY(qAbs(percents.value(Capability_Red) - 0.5) < 0.01);
        QVERIFY(qAbs(percents.value(Capability_Blue) - 0.5) < 0.01);
    }

    // Value scales the whole result down.
    {
        const auto percents = resolveCalibratedPercents(calibration, QColor::fromHsv(0, 255, 128));
        QVERIFY(qAbs(percents.value(Capability_Red) - (128 / 255.0)) < 0.01);
    }

    // Desaturating blends toward the calibrated white point.
    calibration.coolWhite = makeEntry("Cool White", 0.0, Capability_Red, 1.0);
    calibration.coolWhite.channelPercents.insert(Capability_Green, 1.0);
    calibration.coolWhite.channelPercents.insert(Capability_Blue, 1.0);

    {
        const auto percents = resolveCalibratedPercents(calibration, QColor::fromHsv(0, 128, 255));
        const double saturation = 128 / 255.0;
        QVERIFY(qAbs(percents.value(Capability_Red) - 1.0) < 0.01);
        QVERIFY(qAbs(percents.value(Capability_Green) - (1.0 - saturation)) < 0.02);
    }
}

} // namespace photon

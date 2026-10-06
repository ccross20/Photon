#include <QJsonObject>
#include <QTest>
#include "lasertest.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/capability/lasercapability.h"
#include "graph/bus/outputoverridesnode.h"
#include "state/laserstates.h"
#include "state/stateevaluationcontext.h"

namespace photon {

namespace {

// 0-based offsets of the FB4's channels within its 39-channel mode.
enum : uint {
    ChMode = 0, ChMasterIntensity = 1, ChTestFrame = 2, ChGeoSizeX = 3,
    ChPage = 13, ChCue = 14, ChCueSpeed = 15, ChDimmer = 16, ChZoom = 17,
    ChRotation = 25, ChPositionX = 27, ChPositionY = 29, ChScanRate = 31, ChStrobe = 38
};

QString fb4Path()
{
    return QStringLiteral(PHOTON_SOURCE_DIR "/core/resources/fixtures/pangolin-fb4.json");
}

int word(const DMXMatrix &t_matrix, uint t_channel)
{
    return (t_matrix.valueInt(0, t_channel) << 8) | t_matrix.valueInt(0, t_channel + 1);
}

void evaluateState(const StateCapability &t_state, Fixture *t_fixture, DMXMatrix &t_matrix)
{
    StateEvaluationContext context(t_matrix);
    context.fixture = t_fixture;
    t_state.initializeValues(context);
    t_state.evaluate(context);
}

} // namespace

LaserTest::LaserTest(QObject *parent) : QObject{parent}
{
}

void LaserTest::definitionLoads()
{
    Fixture fixture(fb4Path());
    QVERIFY(fixture.isLaser());
    QCOMPARE(fixture.dmxSize(), 39);

    auto *mode = LaserCapability::find(&fixture, LaserCapability::Function_Mode);
    QVERIFY(mode);
    QVERIFY(!mode->is16Bit());

    auto *zoom = LaserCapability::find(&fixture, LaserCapability::Function_Zoom);
    QVERIFY(zoom);
    QVERIFY(zoom->is16Bit());
}

void LaserTest::neutralValues()
{
    Fixture fixture(fb4Path());
    DMXMatrix matrix(1);
    LaserCapability::writeNeutralValues(&fixture, matrix);

    QCOMPARE(word(matrix, ChZoom), 32768);
    QCOMPARE(word(matrix, ChPositionX), 32768);
    QCOMPARE(word(matrix, ChRotation), 0);
    QCOMPARE(matrix.valueInt(0, ChCueSpeed), 100);
    QCOMPARE(matrix.valueInt(0, ChScanRate), 255);
    QCOMPARE(matrix.valueInt(0, ChMode), 0);
}

void LaserTest::disarmedIsSafe()
{
    Fixture fixture(fb4Path());
    DMXMatrix matrix(1);
    matrix.setValue(0, ChMode, 251);
    matrix.setValue(0, ChDimmer, 255);
    matrix.setValue(0, ChPage, 3);

    OutputOverridesNode::applyLaserControl(&fixture, matrix);

    QCOMPARE(matrix.valueInt(0, ChMode), 0);
    QCOMPARE(matrix.valueInt(0, ChDimmer), 0);
    QCOMPARE(matrix.valueInt(0, ChPage), 0);
}

void LaserTest::armedAndSetupModes()
{
    Fixture fixture(fb4Path());

    fixture.setLaserArmed(true);
    DMXMatrix armed(1);
    armed.setValue(0, ChDimmer, 200);
    OutputOverridesNode::applyLaserControl(&fixture, armed);
    QCOMPARE(armed.valueInt(0, ChMode), 251);
    QCOMPARE(armed.valueInt(0, ChDimmer), 200);

    Fixture::LaserSetup setup;
    setup.masterIntensity = 0.5;
    setup.testFrame = 7;
    setup.sizeX = 0.5;
    fixture.setLaserSetup(setup);
    fixture.setLaserSetupActive(true);

    DMXMatrix inSetup(1);
    OutputOverridesNode::applyLaserControl(&fixture, inSetup);
    QCOMPARE(inSetup.valueInt(0, ChMode), 240);
    QCOMPARE(inSetup.valueInt(0, ChMasterIntensity), 128);
    QCOMPARE(inSetup.valueInt(0, ChTestFrame), 7);
    QCOMPARE(word(inSetup, ChGeoSizeX), 49152);

    // Setup takes priority over armed, and leaving it drops back to armed.
    fixture.setLaserSetupActive(false);
    DMXMatrix after(1);
    OutputOverridesNode::applyLaserControl(&fixture, after);
    QCOMPARE(after.valueInt(0, ChMode), 251);
}

void LaserTest::rotationWords()
{
    Fixture fixture(fb4Path());
    LaserRotationState rotation;

    DMXMatrix hold(1);
    evaluateState(rotation, &fixture, hold);
    QCOMPARE(word(hold, ChRotation), 32768);

    rotation.setChannelValue(1, -1.0);
    DMXMatrix fullReverse(1);
    evaluateState(rotation, &fixture, fullReverse);
    QCOMPARE(word(fullReverse, ChRotation), 1);

    rotation.setChannelValue(1, 1.0);
    DMXMatrix fullForward(1);
    evaluateState(rotation, &fixture, fullForward);
    QCOMPARE(word(fullForward, ChRotation), 65535);

    rotation.setChannelValue(2, true);
    DMXMatrix cueRotation(1);
    cueRotation.setValue(0, ChRotation, 99);
    evaluateState(rotation, &fixture, cueRotation);
    QCOMPARE(word(cueRotation, ChRotation), 0);
}

void LaserTest::contentAndStrobe()
{
    Fixture fixture(fb4Path());

    LaserContentState content;
    content.setChannelValue(0, 4);
    content.setChannelValue(1, 12);
    content.setChannelValue(2, 1.5);
    DMXMatrix matrix(1);
    evaluateState(content, &fixture, matrix);
    QCOMPARE(matrix.valueInt(0, ChPage), 4);
    QCOMPARE(matrix.valueInt(0, ChCue), 12);
    QCOMPARE(matrix.valueInt(0, ChCueSpeed), 150);

    LaserStrobeState strobe;
    evaluateState(strobe, &fixture, matrix);
    QCOMPARE(matrix.valueInt(0, ChStrobe), 0);
    strobe.setChannelValue(0, 1.0);
    evaluateState(strobe, &fixture, matrix);
    QCOMPARE(matrix.valueInt(0, ChStrobe), 1);
    strobe.setChannelValue(0, 20.0);
    evaluateState(strobe, &fixture, matrix);
    QCOMPARE(matrix.valueInt(0, ChStrobe), 255);
}

void LaserTest::centeredExtremes()
{
    Fixture fixture(fb4Path());
    LaserPositionState position;

    position.setChannelValue(0, QPointF(-1.0, 0.0));
    DMXMatrix left(1);
    evaluateState(position, &fixture, left);
    QCOMPARE(word(left, ChPositionX), 0);

    position.setChannelValue(0, QPointF(1.0, 0.0));
    DMXMatrix right(1);
    evaluateState(position, &fixture, right);
    QCOMPARE(word(right, ChPositionX), 65535);
    QCOMPARE(word(right, ChPositionY), 32768);   // y = 0 -> centred

    position.setChannelValue(0, QPointF(0.0, -1.0));
    DMXMatrix down(1);
    evaluateState(position, &fixture, down);
    QCOMPARE(word(down, ChPositionX), 32768);
    QCOMPARE(word(down, ChPositionY), 0);
}

void LaserTest::positionPointSaves()
{
    LaserPositionState position;
    position.setChannelValue(0, QPointF(0.25, -0.5));
    QJsonObject json;
    position.writeToJson(json);

    LaserPositionState loaded;
    LoadContext context{nullptr};
    loaded.readFromJson(json, context);
    QCOMPARE(loaded.getChannelValue(0).toPointF(), QPointF(0.25, -0.5));
}

} // namespace photon

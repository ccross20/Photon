#include <QTest>
#include "fixture/capability/fogcapability.h"
#include "fixture/fixture.h"
#include "data/dmxmatrix.h"
#include "fixturecapabilitytest.h"
#include "fixture/capability/fixturecapability.h"

namespace photon {

FixtureCapabilityTest::FixtureCapabilityTest(QObject *parent)
    : QObject{parent}
{

}

void FixtureCapabilityTest::unitTest()
{
    double result;
    FixtureCapability::speed("23Hz", &result);

    QCOMPARE(result, 23);
}

// The bundled one-channel fog machine: a Fog capability on 128-255, so on
// writes 255 and off writes 0.
void FixtureCapabilityTest::fogMachine()
{
    Fixture fixture(QString(PHOTON_SOURCE_DIR) + "/core/resources/fixtures/generic-fog-machine.json");
    QCOMPARE(fixture.dmxSize(), 1);

    const auto fogs = fixture.findCapability(Capability_Fog);
    QCOMPARE(fogs.size(), 1);
    auto *fog = static_cast<FogCapability*>(fogs.first());
    QCOMPARE(int(fog->range().start), 128);

    DMXMatrix matrix(1);
    fog->setOn(true, matrix);
    QCOMPARE(matrix.valueInt(0, 0), 255);
    QVERIFY(fog->isOn(matrix));
    fog->setOn(false, matrix);
    QCOMPARE(matrix.valueInt(0, 0), 0);
    QVERIFY(!fog->isOn(matrix));
}

} // namespace photon

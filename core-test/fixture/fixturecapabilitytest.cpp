#include <QTest>
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

} // namespace photon

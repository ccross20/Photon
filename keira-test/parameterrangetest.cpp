#include <QTest>
#include <QJsonObject>
#include "parameterrangetest.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace keira {

ParameterRangeTest::ParameterRangeTest(QObject *parent) : QObject(parent) {}

// ---- DecimalParameter -------------------------------------------------

void ParameterRangeTest::decimalSoftRangeMirrorsHardByDefault()
{
    DecimalParameter param;
    param.setMinimum(0.0);
    param.setMaximum(10.0);

    // A parameter that never opts into a soft range - the overwhelming
    // majority - must behave exactly as it did before soft bounds existed:
    // its "soft" range just reads back whatever the hard range currently is.
    QVERIFY(!param.hasSoftRange());
    QCOMPARE(param.softMinimum(), 0.0);
    QCOMPARE(param.softMaximum(), 10.0);
}

void ParameterRangeTest::decimalSoftRangeIndependentOfHard()
{
    DecimalParameter param;
    param.setMinimum(-1000.0);
    param.setMaximum(1000.0);
    param.setSoftRange(0.0, 1.0);

    // Hue's case: a wide (here, generous rather than truly infinite) hard
    // ceiling with a tight interactive range inside it.
    QVERIFY(param.hasSoftRange());
    QCOMPARE(param.minimum(), -1000.0);
    QCOMPARE(param.maximum(), 1000.0);
    QCOMPARE(param.softMinimum(), 0.0);
    QCOMPARE(param.softMaximum(), 1.0);
}

void ParameterRangeTest::decimalHardRangeChangeDoesNotClobberExplicitSoftRange()
{
    DecimalParameter param;

    // Soft range set BEFORE the hard range - the reverse of the usual
    // "setRange then narrow" order. Neither order should let one clobber
    // the other.
    param.setSoftRange(0.0, 1.0);
    param.setMinimum(-5.0);
    param.setMaximum(5.0);

    QCOMPARE(param.minimum(), -5.0);
    QCOMPARE(param.maximum(), 5.0);
    QCOMPARE(param.softMinimum(), 0.0);
    QCOMPARE(param.softMaximum(), 1.0);
}

void ParameterRangeTest::decimalJsonRoundTripPreservesSoftRange()
{
    DecimalParameter original;
    original.setMinimum(-1000.0);
    original.setMaximum(1000.0);
    original.setSoftRange(0.0, 1.0);
    original.setPrecision(2);

    QJsonObject json;
    original.writeToJson(json);

    DecimalParameter restored;
    restored.readFromJson(json);

    QVERIFY(restored.hasSoftRange());
    QCOMPARE(restored.minimum(), -1000.0);
    QCOMPARE(restored.maximum(), 1000.0);
    QCOMPARE(restored.softMinimum(), 0.0);
    QCOMPARE(restored.softMaximum(), 1.0);
}

void ParameterRangeTest::decimalJsonOmitsSoftRangeWhenNeverSet()
{
    DecimalParameter original;
    original.setMinimum(0.0);
    original.setMaximum(10.0);

    QJsonObject json;
    original.writeToJson(json);

    // Every parameter that never opts in must keep saved graphs free of
    // softMinimum/softMaximum clutter, not just behave as if unset.
    QVERIFY(!json.contains("softMinimum"));
    QVERIFY(!json.contains("softMaximum"));

    DecimalParameter restored;
    restored.readFromJson(json);
    QVERIFY(!restored.hasSoftRange());
}

// ---- IntegerParameter --------------------------------------------------

void ParameterRangeTest::integerSoftRangeMirrorsHardByDefault()
{
    IntegerParameter param;
    param.setMinimum(0);
    param.setMaximum(100);

    QVERIFY(!param.hasSoftRange());
    QCOMPARE(param.softMinimum(), 0);
    QCOMPARE(param.softMaximum(), 100);
}

void ParameterRangeTest::integerSoftRangeIndependentOfHard()
{
    IntegerParameter param;
    param.setMinimum(0);
    param.setMaximum(1000);
    param.setSoftRange(0, 100);

    QVERIFY(param.hasSoftRange());
    QCOMPARE(param.minimum(), 0);
    QCOMPARE(param.maximum(), 1000);
    QCOMPARE(param.softMinimum(), 0);
    QCOMPARE(param.softMaximum(), 100);
}

void ParameterRangeTest::integerJsonRoundTripPreservesSoftRange()
{
    IntegerParameter original;
    original.setMinimum(0);
    original.setMaximum(1000);
    original.setSoftRange(0, 100);

    QJsonObject json;
    original.writeToJson(json);

    IntegerParameter restored;
    restored.readFromJson(json);

    QVERIFY(restored.hasSoftRange());
    QCOMPARE(restored.minimum(), 0);
    QCOMPARE(restored.maximum(), 1000);
    QCOMPARE(restored.softMinimum(), 0);
    QCOMPARE(restored.softMaximum(), 100);
}

} // namespace keira

#include <QTest>
#include <QJsonObject>
#include "propertyaddresstest.h"
#include "gui/properties/propertyaddress.h"

namespace photon {

namespace {

// "Routine > Canvas > Noise" - a node two graphs deep, the common case.
PropertyAddress makeNodeAddress()
{
    PropertyAddress address;
    address.append(PropertyAddress::KindGraph, "graph-root", "Routine");
    address.append(PropertyAddress::KindGraph, "graph-sub", "Canvas");
    address.append(PropertyAddress::KindNode, "node-1", "Noise");
    return address;
}

} // namespace

PropertyAddressTest::PropertyAddressTest(QObject *parent) : QObject(parent) {}

void PropertyAddressTest::emptyAddressHasNoSegments()
{
    PropertyAddress address;
    QVERIFY(address.isEmpty());
    QCOMPARE(address.size(), 0);
    QVERIFY(address.toString().isEmpty());
    // A leaf read off an empty address must be a harmless default, not a crash -
    // the panel asks for it before anything is selected.
    QVERIFY(address.last().id.isEmpty());
}

void PropertyAddressTest::segmentsPreserveOrder()
{
    const PropertyAddress address = makeNodeAddress();
    QCOMPARE(address.size(), 3);
    QCOMPARE(address.segments().at(0).id, QByteArray("graph-root"));
    QCOMPARE(address.segments().at(1).id, QByteArray("graph-sub"));
    QCOMPARE(address.segments().at(2).id, QByteArray("node-1"));
}

void PropertyAddressTest::toStringJoinsIds()
{
    QCOMPARE(makeNodeAddress().toString(), QStringLiteral("graph-root/graph-sub/node-1"));
}

void PropertyAddressTest::toDisplayStringJoinsLabels()
{
    QCOMPARE(makeNodeAddress().toDisplayString(), QStringLiteral("Routine / Canvas / Noise"));
}

void PropertyAddressTest::jsonRoundTripPreservesSegments()
{
    const PropertyAddress original = makeNodeAddress();

    QJsonObject json;
    original.writeToJson(json);

    PropertyAddress restored;
    restored.readFromJson(json);

    QCOMPARE(restored.size(), original.size());
    QCOMPARE(restored, original);
    // Labels ride along so a pinned tab whose target has gone can still be drawn.
    QCOMPARE(restored.toDisplayString(), original.toDisplayString());
}

void PropertyAddressTest::equalityIgnoresLabels()
{
    PropertyAddress a = makeNodeAddress();

    PropertyAddress renamed;
    renamed.append(PropertyAddress::KindGraph, "graph-root", "Routine");
    renamed.append(PropertyAddress::KindGraph, "graph-sub", "Canvas");
    renamed.append(PropertyAddress::KindNode, "node-1", "Noise (renamed)");

    // Renaming a node must not orphan its pinned tab.
    QCOMPARE(a, renamed);
}

void PropertyAddressTest::equalityDistinguishesIdsAndKinds()
{
    const PropertyAddress base = makeNodeAddress();

    PropertyAddress otherId;
    otherId.append(PropertyAddress::KindGraph, "graph-root", "Routine");
    otherId.append(PropertyAddress::KindGraph, "graph-sub", "Canvas");
    otherId.append(PropertyAddress::KindNode, "node-2", "Noise");
    QVERIFY(base != otherId);

    // Same id under a different kind is a different thing - ids are only
    // unique within their own domain.
    PropertyAddress otherKind;
    otherKind.append(PropertyAddress::KindGraph, "graph-root", "Routine");
    otherKind.append(PropertyAddress::KindGraph, "graph-sub", "Canvas");
    otherKind.append(PropertyAddress::KindGizmo, "node-1", "Noise");
    QVERIFY(base != otherKind);

    // A prefix is not the same address as the full path.
    PropertyAddress shorter;
    shorter.append(PropertyAddress::KindGraph, "graph-root", "Routine");
    QVERIFY(base != shorter);
}

void PropertyAddressTest::lastReturnsLeafSegment()
{
    const PropertyAddress address = makeNodeAddress();
    QCOMPARE(address.last().kind, PropertyAddress::KindNode);
    QCOMPARE(address.last().id, QByteArray("node-1"));
    QCOMPARE(address.last().label, QStringLiteral("Noise"));
}

} // namespace photon

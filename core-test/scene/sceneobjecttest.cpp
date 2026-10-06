#include <QJsonObject>
#include <QTest>
#include "sceneobjecttest.h"
#include "scene/sceneobject.h"
#include "scene/scenebox.h"
#include "scene/scenefactory.h"
#include "project/project.h"

namespace photon {

SceneObjectTest::SceneObjectTest(QObject *parent)
    : QObject{parent}
{
}

// Reproduces a Project panel crash: make a fixture, make a group as its
// child, drag the group up to root, then drag the fixture in under the group.
//
// addChild() relays a child's descendantModified signal back up through its
// new parent's own descendantModified. removeChild() must tear down that
// exact relay, or the child keeps a live connection back to its old parent
// after being moved elsewhere. Here that means: group's time as fixture's
// child wires group->fixture; moving group to root doesn't touch that relay
// (it's now stale - group isn't fixture's child anymore); re-parenting
// fixture under group then wires a fresh fixture->group relay. With both
// relays live, any metadata change bounces fixture -> group -> fixture forever
// and blows the stack.
void SceneObjectTest::reparentingDoesNotLeakSignalRelays()
{
    SceneObject root("root");

    auto *fixture = new SceneObject("fixture");
    fixture->setParentSceneObject(&root);

    auto *group = new SceneObject("group");
    group->setParentSceneObject(fixture);

    group->setParentSceneObject(&root);     // move group up to root
    fixture->setParentSceneObject(group);   // drag fixture under group

    // Recursed until the stack overflowed here before the fix.
    fixture->setName("Renamed Fixture");

    QCOMPARE(fixture->name(), QString("Renamed Fixture"));
}

void SceneObjectTest::nextAvailableName_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QStringList>("taken");
    QTest::addColumn<QString>("expected");

    QTest::newRow("unnumbered") << "Spot" << QStringList{"Spot"} << "Spot 2";
    QTest::newRow("past highest") << "Spot" << QStringList{"Spot", "Spot 2", "Spot 5"} << "Spot 6";
    QTest::newRow("numbered copy") << "Spot 2" << QStringList{"Spot", "Spot 2", "Spot 3"} << "Spot 4";
    QTest::newRow("only numbered") << "Wash 1" << QStringList{"Wash 1"} << "Wash 2";
    QTest::newRow("zero padded") << "Par 07" << QStringList{"Par 07"} << "Par 08";
    QTest::newRow("no separator") << "Bar3" << QStringList{"Bar3"} << "Bar4";
    QTest::newRow("other stems ignored") << "Spot" << QStringList{"Spot", "Spotlight 9", "Spot Left 4"} << "Spot 2";
    QTest::newRow("just a number") << "12" << QStringList{"12", "4"} << "13";
}

void SceneObjectTest::nextAvailableName()
{
    QFETCH(QString, name);
    QFETCH(QStringList, taken);
    QFETCH(QString, expected);

    const QSet<QString> takenSet(taken.begin(), taken.end());
    QCOMPARE(SceneObject::nextAvailableName(name, takenSet), expected);
}

void SceneObjectTest::boxSizeSaves()
{
    SceneBox box;
    box.setSize(QVector3D(2.0f, 0.5f, 3.0f));
    box.setColor(QColor(10, 20, 30));

    QJsonObject json;
    box.writeToJson(json);

    std::unique_ptr<SceneObject> loaded(SceneFactory::createObject(json.value("typeId").toString().toLatin1()));
    QVERIFY(dynamic_cast<SceneBox*>(loaded.get()));
    loaded->readFromJson(json, LoadContext{});

    auto *loadedBox = static_cast<SceneBox*>(loaded.get());
    QCOMPARE(loadedBox->size(), QVector3D(2.0f, 0.5f, 3.0f));
    QCOMPARE(loadedBox->color(), QColor(10, 20, 30));

    // Sides never collapse to zero (or flip negative).
    box.setSize(QVector3D(0.0f, -1.0f, 1.0f));
    QCOMPARE(box.size(), QVector3D(SceneBox::MinimumSide, SceneBox::MinimumSide, 1.0f));
}

} // namespace photon

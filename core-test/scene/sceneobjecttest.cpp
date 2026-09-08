#include <QTest>
#include "sceneobjecttest.h"
#include "scene/sceneobject.h"

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

} // namespace photon

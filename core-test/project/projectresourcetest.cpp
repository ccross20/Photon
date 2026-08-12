#include <QTest>
#include <QJsonObject>
#include <QSignalSpy>
#include "projectresourcetest.h"
#include "project/projectresource.h"
#include "scene/sceneobject.h"
#include "fixture/fixturegroup.h"
#include "sequence/sequence.h"
#include "project/project.h"

namespace photon {

ProjectResourceTest::ProjectResourceTest(QObject *parent)
    : QObject{parent}
{
}

void ProjectResourceTest::tagsAreNormalisedAndSorted()
{
    SceneObject object("group");

    object.setResourceTags({"Zebra", "  apple  ", "APPLE", "", "mid"});

    // Lower-cased, trimmed, de-duplicated, sorted.
    QCOMPARE(object.resourceTags(), QStringList({"apple", "mid", "zebra"}));

    object.addResourceTag("Beta");
    QCOMPARE(object.resourceTags(), QStringList({"apple", "beta", "mid", "zebra"}));

    // Adding an existing tag in a different case is a no-op.
    object.addResourceTag("BETA");
    QCOMPARE(object.resourceTags(), QStringList({"apple", "beta", "mid", "zebra"}));

    // Removal is case-insensitive too - the old SceneObject::removeTag checked
    // the lower-cased tag but then removed the raw one, so a mixed-case call
    // silently did nothing.
    object.removeResourceTag("ZEBRA");
    QCOMPARE(object.resourceTags(), QStringList({"apple", "beta", "mid"}));

    QVERIFY(object.hasResourceTag("Apple"));
    QVERIFY(!object.hasResourceTag("zebra"));
}

void ProjectResourceTest::unchangedTagsDoNotNotify()
{
    SceneObject object("group");
    object.setResourceTags({"one", "two"});

    QSignalSpy spy(object.resourceNotifier(), &ProjectResourceNotifier::resourceChanged);

    // Same set, different order and case - normalises to the identical list, so
    // nothing should fire.
    object.setResourceTags({"TWO", "one"});
    QCOMPARE(spy.count(), 0);

    object.removeResourceTag("absent");
    QCOMPARE(spy.count(), 0);

    object.addResourceTag("three");
    QCOMPARE(spy.count(), 1);
}

void ProjectResourceTest::tagsRoundTripThroughJson()
{
    SceneObject source("group");
    source.setResourceTags({"front", "wash"});

    QJsonObject json;
    source.writeResourceJson(json);

    SceneObject target("group");
    target.readResourceJson(json);

    QCOMPARE(target.resourceTags(), QStringList({"front", "wash"}));

    // An untagged resource writes no key at all rather than an empty array.
    QJsonObject empty;
    SceneObject("group").writeResourceJson(empty);
    QVERIFY(!empty.contains("tags"));
}

void ProjectResourceTest::sceneObjectBridgesBothNotificationPaths()
{
    SceneObject object("group");

    QSignalSpy metadataSpy(&object, &SceneObject::metadataChanged);
    QSignalSpy resourceSpy(object.resourceNotifier(), &ProjectResourceNotifier::resourceChanged);

    // A tag edit enters through ProjectResource and must surface on the scene's
    // own signal as well - SceneObject overrides notifyResourceChanged() to
    // redirect into metadataChanged, which is bridged back out. Exactly one of
    // each: if the bridge fed itself this would recurse.
    object.addResourceTag("mover");
    QCOMPARE(metadataSpy.count(), 1);
    QCOMPARE(resourceSpy.count(), 1);

    // A rename enters through the scene side and must reach the resource side.
    object.setName("Renamed");
    QCOMPARE(metadataSpy.count(), 2);
    QCOMPARE(resourceSpy.count(), 2);

    // ...and so must a visibility toggle.
    object.setVisible(false);
    QCOMPARE(metadataSpy.count(), 3);
    QCOMPARE(resourceSpy.count(), 3);
}

void ProjectResourceTest::sceneObjectTagAliasesShareStorage()
{
    SceneObject object("group");

    // tags()/addTag()/removeTag() are aliases kept for existing scene call
    // sites; they must operate on the same storage as the resource API.
    object.addTag("Truss");
    QCOMPARE(object.tags(), QStringList({"truss"}));
    QCOMPARE(object.resourceTags(), object.tags());

    object.setResourceTags({"a", "b"});
    QCOMPARE(object.tags(), QStringList({"a", "b"}));

    object.removeTag("a");
    QCOMPARE(object.resourceTags(), QStringList({"b"}));
}

void ProjectResourceTest::fixtureGroupIsAnIdentifiedResource()
{
    FixtureGroupCollection collection;

    FixtureGroup *group = collection.addGroup("Front Wash");
    QVERIFY(group);
    QVERIFY(!group->uniqueId().isEmpty());
    QCOMPARE(group->resourceId(), group->uniqueId());
    QCOMPARE(group->resourceTypeId(), QByteArray("group"));
    QCOMPARE(group->resourceName(), QStringLiteral("Front Wash"));

    QCOMPARE(collection.findGroupWithId(group->uniqueId()), group);

    // Groups are taggable now, like every other resource.
    group->addResourceTag("act1");
    QVERIFY(group->hasResourceTag("act1"));

    QJsonObject json;
    group->writeToJson(json);

    FixtureGroup restored;
    restored.readFromJson(json);
    QCOMPARE(restored.uniqueId(), group->uniqueId());
    QCOMPARE(restored.name(), QStringLiteral("Front Wash"));
    QCOMPARE(restored.resourceTags(), QStringList({"act1"}));
}

void ProjectResourceTest::fixtureGroupQueryChangeReachesCollection()
{
    FixtureGroupCollection collection;
    FixtureGroup *group = collection.addGroup("Movers");

    QSignalSpy changedSpy(&collection, &FixtureGroupCollection::groupChanged);

    FixtureQuery query;
    query.type = "mover";
    group->setQuery(query);

    // The collection relays the group's own signal, so editing a group no
    // longer needs a separate notifyChanged() call from the UI.
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.first().first().toInt(), 0);
    QCOMPARE(group->query().type, QStringLiteral("mover"));

    // Renaming reaches it the same way.
    group->setName("Movers 2");
    QCOMPARE(changedSpy.count(), 2);
}

void ProjectResourceTest::sequenceIdRoundTrips()
{
    Sequence sequence("Opener");
    QVERIFY(!sequence.uniqueId().isEmpty());
    QCOMPARE(sequence.resourceId(), sequence.uniqueId());
    QCOMPARE(sequence.resourceTypeId(), QByteArray("sequence"));

    sequence.addResourceTag("show");

    QJsonObject json;
    sequence.writeToJson(json);

    LoadContext context;
    Sequence restored;
    restored.readFromJson(json, context);

    QCOMPARE(restored.uniqueId(), sequence.uniqueId());
    QCOMPARE(restored.name(), QStringLiteral("Opener"));
    QCOMPARE(restored.resourceTags(), QStringList({"show"}));
}

void ProjectResourceTest::selectionCarriesAnyResourceType()
{
    Project project;
    auto *group = project.groups()->addGroup("Front");

    QSignalSpy resourceSpy(&project, &Project::selectedResourceChanged);

    // A fixture group is not a SceneObject - the old selection API could not
    // have carried it at all.
    project.setSelectedResource(group);

    QCOMPARE(project.selectedResource(), static_cast<ProjectResource*>(group));
    QCOMPARE(project.selectedResources().size(), 1);
    QCOMPARE(resourceSpy.count(), 1);
}

void ProjectResourceTest::sceneConsumersSeeOnlyTheirSlice()
{
    Project project;

    auto *object = new SceneObject("group");
    object->setParentSceneObject(project.sceneRoot());
    auto *group = project.groups()->addGroup("Front");

    // Mixed selection: the scene-facing accessors report only the scene part.
    project.setSelectedResources({group, object});

    QCOMPARE(project.selectedResources().size(), 2);
    QCOMPARE(project.selectedSceneObjects(), QList<SceneObject*>({object}));
    // Primary is the last of the *whole* selection, but the scene primary is
    // the last scene object.
    QCOMPARE(project.selectedResource(), static_cast<ProjectResource*>(object));
    QCOMPARE(project.selectedSceneObject(), object);

    QSignalSpy sceneSpy(&project, &Project::selectedSceneObjectChanged);

    // Selecting a non-scene resource must clear the scene selection rather than
    // leave the visualizer highlighting something no longer selected.
    project.setSelectedResource(group);

    QVERIFY(project.selectedSceneObjects().isEmpty());
    QCOMPARE(project.selectedSceneObject(), nullptr);
    QCOMPARE(sceneSpy.count(), 1);
    QCOMPARE(sceneSpy.first().first().value<SceneObject*>(), nullptr);
}

void ProjectResourceTest::sceneSelectionSignalsStayQuietWhenTheSliceIsUnchanged()
{
    Project project;

    auto *object = new SceneObject("group");
    object->setParentSceneObject(project.sceneRoot());
    auto *groupA = project.groups()->addGroup("A");
    auto *groupB = project.groups()->addGroup("B");

    project.setSelectedResources({object, groupA});

    QSignalSpy sceneSpy(&project, &Project::selectedSceneObjectsChanged);
    QSignalSpy resourceSpy(&project, &Project::selectedResourcesChanged);

    // Swapping which group is selected leaves the scene slice identical, so
    // scene consumers shouldn't be churned.
    project.setSelectedResources({object, groupB});

    QCOMPARE(resourceSpy.count(), 1);
    QCOMPARE(sceneSpy.count(), 0);
}

} // namespace photon

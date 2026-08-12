#include <QTest>
#include "tagtest.h"
#include "gui/tag/tagcolor.h"
#include "project/project.h"
#include "scene/sceneobject.h"
#include "fixture/fixturegroup.h"
#include "routine/routine.h"
#include "routine/routinecollection.h"
#include "surface/surface.h"
#include "pixel/pixellayout.h"
#include "pixel/pixellayoutcollection.h"

namespace photon {

TagTest::TagTest(QObject *parent)
    : QObject{parent}
{
}

void TagTest::tagColorIsDeterministic()
{
    const QColor first = tagColor("wash");
    const QColor second = tagColor("wash");

    QCOMPARE(first, second);

    // Different tags should (overwhelmingly likely) get different colors -
    // not a strict guarantee for arbitrary strings, but true for two
    // ordinary, unrelated tag names, and worth catching a "always returns the
    // same color" regression.
    QVERIFY(tagColor("wash") != tagColor("mover"));
}

void TagTest::tagColorIsCaseInsensitive()
{
    // Tags are stored lower-cased everywhere (ProjectResource normalises on
    // write), but a chip should still color the same regardless of how it's
    // asked for.
    QCOMPARE(tagColor("Wash"), tagColor("wash"));
    QCOMPARE(tagColor("  wash  "), tagColor("wash"));
}

void TagTest::tagTextColorContrastsBackground()
{
    QCOMPARE(tagTextColor(QColor(Qt::white)), QColor(Qt::black));
    QCOMPARE(tagTextColor(QColor(Qt::black)), QColor(Qt::white));
}

void TagTest::allTagsUnionsEveryResourceType()
{
    Project project;

    auto *object = new SceneObject("group");
    object->setParentSceneObject(project.sceneRoot());
    object->addResourceTag("front");

    auto *group = project.groups()->addGroup("Movers");
    group->addResourceTag("act1");

    auto *routine = new Routine("Chase");
    routine->addResourceTag("intro");
    project.routines()->addRoutine(routine);

    auto *layout = new PixelLayout;
    layout->addResourceTag("matrix");
    project.pixelLayouts()->addLayout(layout);

    const QStringList all = project.allTags();

    QVERIFY(all.contains("front"));
    QVERIFY(all.contains("act1"));
    QVERIFY(all.contains("intro"));
    QVERIFY(all.contains("matrix"));
}

void TagTest::allTagsIsSortedAndDeduped()
{
    Project project;

    auto *a = new SceneObject("group");
    a->setParentSceneObject(project.sceneRoot());
    a->addResourceTag("wash");

    auto *b = new SceneObject("group");
    b->setParentSceneObject(project.sceneRoot());
    b->addResourceTag("wash");   // same tag as a - must not appear twice
    b->addResourceTag("arc");

    const QStringList all = project.allTags();

    QCOMPARE(all.count("wash"), 1);

    QStringList sorted = all;
    sorted.sort();
    QCOMPARE(all, sorted);
}

} // namespace photon

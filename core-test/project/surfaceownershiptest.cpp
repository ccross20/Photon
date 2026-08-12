#include <QTest>
#include <QJsonArray>
#include <QJsonObject>
#include "surfaceownershiptest.h"
#include "project/project.h"
#include "surface/surface.h"
#include "surface/surfacecollection.h"
#include "graph/bus/busgraph.h"
#include "graph/bus/surfacenode.h"

namespace photon {

namespace {

// The bus's Surface node, located by type rather than by name so the test
// doesn't break if the default node is renamed.
SurfaceNode *busSurfaceNode(Project &project)
{
    for(auto *node : project.bus()->nodes())
        if(auto *surfaceNode = dynamic_cast<SurfaceNode*>(node))
            return surfaceNode;
    return nullptr;
}

} // namespace

SurfaceOwnershipTest::SurfaceOwnershipTest(QObject *parent)
    : QObject{parent}
{
}

void SurfaceOwnershipTest::newProjectSeedsOneSurfaceBoundToTheBusNode()
{
    Project project;

    QCOMPARE(project.surfaces()->surfaceCount(), 1);

    auto *node = busSurfaceNode(project);
    QVERIFY(node);

    // The node holds an id, not the object.
    QVERIFY(!node->surfaceId().isEmpty());
    QCOMPARE(node->surfaceId(), project.surfaces()->surfaceAtIndex(0)->uniqueId());
}

void SurfaceOwnershipTest::collectionRevisionTracksMembership()
{
    Project project;
    auto *surfaces = project.surfaces();

    const quint32 before = surfaces->revision();

    auto *extra = new Surface("Second");
    surfaces->addSurface(extra);
    QVERIFY(surfaces->revision() != before);

    // Re-adding the same surface is a no-op and must not churn the revision,
    // or every referencing node would re-resolve for nothing.
    const quint32 afterAdd = surfaces->revision();
    surfaces->addSurface(extra);
    QCOMPARE(surfaces->revision(), afterAdd);

    surfaces->removeSurface(extra);
    QVERIFY(surfaces->revision() != afterAdd);
}

void SurfaceOwnershipTest::removedSurfaceStopsResolving()
{
    Project project;
    auto *surfaces = project.surfaces();

    auto *extra = new Surface("Doomed");
    const QByteArray id = extra->uniqueId();
    surfaces->addSurface(extra);
    QCOMPARE(surfaces->surfaceCount(), 2);
    QCOMPARE(surfaces->findSurfaceWithId(id), extra);

    surfaces->removeSurface(extra);

    // What a referencing SurfaceNode sees: the id no longer resolves, which is
    // the case its null guard covers. (The surface object itself is destroyed
    // via deleteLater, so it outlives this call.)
    QCOMPARE(surfaces->surfaceCount(), 1);
    QVERIFY(!surfaces->findSurfaceWithId(id));
}

void SurfaceOwnershipTest::nodeSerializesOnlyItsSurfaceId()
{
    SurfaceNode node;
    node.createParameters();
    node.setSurfaceId("surface-abc");

    QJsonObject json;
    node.writeToJson(json);

    QCOMPARE(json.value("surfaceId").toString(), QStringLiteral("surface-abc"));
    // The surface itself belongs to the project now, not to the node's json.
    QVERIFY(!json.contains("surface"));

    SurfaceNode restored;
    restored.createParameters();
    restored.readFromJson(json, nullptr);
    QCOMPARE(restored.surfaceId(), QByteArray("surface-abc"));
}

void SurfaceOwnershipTest::projectWritesSurfacesAtTopLevel()
{
    Project project;
    project.surfaces()->addSurface(new Surface("Second"));

    QJsonObject saved;
    project.writeToJson(saved);

    QVERIFY(saved.contains("surfaces"));
    QCOMPARE(saved.value("surfaces").toArray().count(), 2);
}

void SurfaceOwnershipTest::readReplacesSeededSurfaces()
{
    QJsonArray surfaceArray;
    for(const QString &name : {QStringLiteral("Alpha"), QStringLiteral("Beta")})
    {
        Surface surface(name);
        QJsonObject surfaceObj;
        surface.writeToJson(surfaceObj);
        surfaceArray.append(surfaceObj);
    }

    // Deliberately no "bus" key: Project::readFromJson only reaches for the
    // node library when there is a bus to rebuild, so this stays runnable
    // without a live PhotonCore.
    QJsonObject json;
    json.insert("surfaces", surfaceArray);

    Project project;
    QCOMPARE(project.surfaces()->surfaceCount(), 1);   // constructor's default

    project.readFromJson(json);

    // The seeded default is dropped rather than left orphaned beside the
    // loaded pair.
    QCOMPARE(project.surfaces()->surfaceCount(), 2);
    QCOMPARE(project.surfaces()->surfaceAtIndex(0)->name(), QStringLiteral("Alpha"));
    QCOMPARE(project.surfaces()->surfaceAtIndex(1)->name(), QStringLiteral("Beta"));
}

} // namespace photon

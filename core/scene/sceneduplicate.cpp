#include <QHash>
#include <QSet>
#include "sceneduplicate.h"
#include "sceneobject.h"
#include "sceneiterator.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"

namespace photon {

int nextAvailableDMXOffset(int t_universe)
{
    int next = 0;
    for(auto *fixture : photonApp->project()->fixtures()->fixtures())
    {
        if(fixture->universe() != t_universe)
            continue;
        const int end = fixture->dmxOffset() + fixture->dmxSize();
        if(end > next)
            next = end;
    }
    return next;
}

QVector<SceneObject*> duplicateSceneObjects(const QVector<SceneObject*> &t_objects)
{
    QHash<int, int> nextDMXByUniverse;
    auto claimNextDMX = [&nextDMXByUniverse](int t_universe, int t_size) {
        auto it = nextDMXByUniverse.find(t_universe);
        if(it == nextDMXByUniverse.end())
            it = nextDMXByUniverse.insert(t_universe, nextAvailableDMXOffset(t_universe));
        const int offset = it.value();
        it.value() += t_size;
        return offset;
    };

    // Copies made in this same pass are added as they're named so a
    // multi-selection doesn't hand two of them the same number.
    QSet<QString> takenNames;
    if(auto *root = photonApp->project()->sceneRoot())
    {
        for(auto *object : SceneIterator::ToList(root))
            takenNames.insert(object->name());
    }

    const QSet<SceneObject*> selected(t_objects.begin(), t_objects.end());
    auto hasSelectedAncestor = [&selected](SceneObject *t_object) {
        for(auto *parent = t_object->parentSceneObject(); parent; parent = parent->parentSceneObject())
        {
            if(selected.contains(parent))
                return true;
        }
        return false;
    };

    QVector<SceneObject*> clones;
    for(auto *object : t_objects)
    {
        if(!object || hasSelectedAncestor(object))
            continue;

        SceneObject *clone = object->clone();
        if(!clone)
            continue;

        const QString name = SceneObject::nextAvailableName(object->name(), takenNames);
        clone->setName(name);
        takenNames.insert(name);

        clone->setParentSceneObject(object->parentSceneObject());
        clones.append(clone);

        auto clonedFixtures = SceneIterator::FindMany(clone, [](SceneObject *t_candidate, bool *){
            return dynamic_cast<Fixture*>(t_candidate) != nullptr;
        });
        for(auto *candidate : clonedFixtures)
        {
            auto *fixture = static_cast<Fixture*>(candidate);
            fixture->setDMXOffset(claimNextDMX(fixture->universe(), fixture->dmxSize()));
        }
    }
    return clones;
}

} // namespace photon

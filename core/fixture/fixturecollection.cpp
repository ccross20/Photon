#include "fixturecollection.h"
#include "fixture.h"
#include "photoncore.h"
#include "scene/sceneiterator.h"
#include "project/project.h"

namespace photon {

class FixtureCollection::Impl
{
public:
    QVector<Fixture*> fixtures;
};

FixtureCollection::FixtureCollection(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

FixtureCollection::~FixtureCollection(){
    for(auto fixture : m_impl->fixtures)
        delete fixture;
    delete m_impl;
}

Fixture* FixtureCollection::fixtureById(const QByteArray &t_id)
{
    return dynamic_cast<Fixture*>(SceneIterator::FindOne(photonApp->project()->sceneRoot(),[t_id](SceneObject *t_obj){return t_obj->uniqueId() == t_id;}));
}

QVector<Fixture*> FixtureCollection::fixturesWithName(const QString &t_text)
{
    QVector<Fixture*> results;

    auto collection = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[t_text](SceneObject *t_obj, bool *t_continue){
                           *t_continue = true;
                           return t_obj->name() == t_text;
                       });

    for(auto item : collection)
        results.append(static_cast<Fixture*>(item));

    return results;

}

Fixture *FixtureCollection::fixtureWithId(const QByteArray &t_id) const
{
    return dynamic_cast<Fixture*>(SceneIterator::FindOne(photonApp->project()->sceneRoot(),[t_id](SceneObject *t_obj){
                                      if(t_obj->typeId() == "fixture")
                                        return static_cast<Fixture*>(t_obj)->uniqueId() == t_id;
                                      return false;
                                  }));
}

const QVector<Fixture*> FixtureCollection::fixtures() const
{
    auto items = SceneIterator::ToList(photonApp->project()->sceneRoot());

    QVector<Fixture*> toReturn;

    for(auto item : items)
    {
        if(item->typeId() == "fixture")
            toReturn.append(static_cast<Fixture*>(item));
    }


    return toReturn;
}

} // namespace photon

#ifndef SCENEITERATOR_H
#define SCENEITERATOR_H


#include "photon-global.h"
#include "sceneobject.h"

namespace photon {

class SceneIterator
{
public:
    SceneIterator(){

    }

    static QVector<SceneObject*> ToList(SceneObject*t_root)
    {
        QVector<SceneObject*> collection;
        recursiveCollect(t_root, collection);
        return collection;
    };

    static void recursiveCollect(SceneObject *t_item, QVector<SceneObject*> &t_collection)
    {
        t_collection.append(t_item);
        for(auto obj : t_item->sceneChildren())
        {
            recursiveCollect(obj, t_collection);
        }
    }

    static SceneObject* FindOne(SceneObject*t_root, std::function<bool(SceneObject*)> t_callback)
    {
        return recursiveFindOne(t_root, t_callback);
    };

    static SceneObject *recursiveFindOne(SceneObject *t_item, std::function<bool(SceneObject*)> t_callback)
    {
        if(t_callback(t_item))
            return t_item;
        for(auto obj : t_item->sceneChildren())
        {
            auto result = recursiveFindOne(obj, t_callback);
            if(result)
                return result;
        }

        return nullptr;
    }

    static QVector<SceneObject*> FindMany(SceneObject*t_root, std::function<bool(SceneObject*, bool *)> t_callback)
    {
        QVector<SceneObject*> collection;
        recursiveFindMany(t_root, t_callback, collection);
        return collection;
    };

    static void recursiveFindMany(SceneObject *t_item, std::function<bool(SceneObject*, bool *)> t_callback, QVector<SceneObject*> &t_collection)
    {
        bool continueDown;
        if(t_callback(t_item, &continueDown))
            t_collection.append(t_item);

        if(continueDown)
        {
            for(auto obj : t_item->sceneChildren())
            {
                recursiveFindMany(obj, t_callback, t_collection);
            }
        }
    }

};


}

#endif // SCENEITERATOR_H

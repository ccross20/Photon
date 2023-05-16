#include "treemaskeffect.h"
#include "photoncore.h"
#include "project/project.h"
#include "scene/sceneiterator.h"
#include "fixture/fixture.h"

namespace photon {


SelectorWatcher::SelectorWatcher(TreeMaskEffect *t_effect, SceneObjectSelector *t_selector):QObject(t_selector),m_effect(t_effect),m_selector(t_selector)
{
    connect(m_selector, &SceneObjectSelector::selectionChanged,this, &SelectorWatcher::selectionChanged);

}

void SelectorWatcher::selectionChanged()
{
    m_effect->setSelection(m_selector->selectedObjects());
}

TreeMaskEffect::TreeMaskEffect() : MaskEffect("photon.core.tree-mask-effect")
{

}

QVector<Fixture*> TreeMaskEffect::process(const QVector<Fixture*> fixtures) const
{
    QVector<Fixture*> results;
    for(const auto &objectId : m_selectedIds)
    {
        auto foundObj = SceneIterator::FindOne(photonApp->project()->sceneRoot(),[objectId](SceneObject *t_sceneObject){
           return t_sceneObject->uniqueId() == objectId;
        });

        if(foundObj)
        {
            for(auto obj : SceneIterator::ToList(foundObj))
            {
                if(obj->typeId() == "fixture")
                    results.append(static_cast<Fixture*>(obj));
            }
        }
    }
    return results;
}

QWidget *TreeMaskEffect::createEditor()
{
    auto selector = new SceneObjectSelector(photonApp->project()->sceneRoot());

    QVector<SceneObject *> selected;
    for(const auto &id : m_selectedIds)
    {
        auto foundObj = SceneIterator::FindOne(photonApp->project()->sceneRoot(),[id](SceneObject *t_sceneObject){
           return t_sceneObject->uniqueId() == id;
        });
        if(foundObj)
            selected << foundObj;
    }

    selector->setSelectedObjects(selected);
    new SelectorWatcher(this,selector);

    return selector;
}

void TreeMaskEffect::setSelection(const QVector<SceneObject *> &t_selection)
{
    m_selectedIds.clear();

    for(auto obj : t_selection)
        m_selectedIds.append(obj->uniqueId());

    updated();

}

void TreeMaskEffect::readFromJson(const QJsonObject &t_json)
{
    MaskEffect::readFromJson(t_json);

    m_selectedIds.clear();
    auto idArray = t_json.value("ids").toArray();
    for(const auto &id : idArray)
        m_selectedIds.append(id.toString().toLatin1());
}

void TreeMaskEffect::writeToJson(QJsonObject &t_json) const
{
    MaskEffect::writeToJson(t_json);

    QJsonArray idArray;
    for(const auto &id : m_selectedIds)
        idArray.append(QString(id));

    t_json.insert("ids", idArray);

}

MaskEffectInformation TreeMaskEffect::info()
{
    MaskEffectInformation toReturn([](){return new TreeMaskEffect;});
    toReturn.name = "Tree Mask";
    toReturn.effectId = "photon.core.tree-mask-effect";

    return toReturn;
}


} // namespace photon

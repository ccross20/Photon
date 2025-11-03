#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include "actionstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/fixtureclip.h"
#include "falloff/falloffeffect.h"
#include "fixture/maskeffect.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "gui/menufactory.h"
#include "state/state.h"
#include "surface/surfaceaction.h"
#include "surface/fixtureaction.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "surface/surfacegizmocontainer.h"

namespace photon {

ActionTreeView::ActionTreeView() : QTreeView()
{

}

void ActionTreeView::mousePressEvent(QMouseEvent *event)
{
    auto item = indexAt(event->pos());
    auto itemData = static_cast<ActionModel*>(model())->dataForIndex(item);
    CreateData *createData = dynamic_cast<CreateData*>(itemData);

    if(createData)
    {
        auto parentData = createData->parent();

        if(dynamic_cast<ChannelData*>(parentData))
        {
            Channel *channel = dynamic_cast<ChannelData*>(parentData)->channel();

            MenuFactory<EffectInformation> factory;

            auto effects = photonApp->plugins()->channelEffects();
            for(auto &info : effects)
            {
                factory.addItem(info.categories, info);
            }

            EffectInformation selectedInfo;
            if(factory.showMenu(event->globalPosition().toPoint(), selectedInfo))
            {
                auto effect = photonApp->plugins()->createChannelEffect(selectedInfo.effectId);

                if(effect)
                {
                    channel->addEffect(effect);

                    auto effectData = dynamic_cast<ChannelData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ActionModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }

        }
        else if(dynamic_cast<ActionFalloffData*>(parentData))
        {
            MenuFactory<FalloffEffectInformation> factory;

            auto effects = photonApp->plugins()->falloffEffects();
            for(auto &info : effects)
            {
                factory.addItem(info.categories, info);
            }

            FalloffEffectInformation selectedInfo;
            if(factory.showMenu(event->globalPosition().toPoint(), selectedInfo))
            {

                FixtureAction *clip = dynamic_cast<ActionFalloffData*>(parentData)->action();

                auto effect = photonApp->plugins()->createFalloffEffect(selectedInfo.effectId);

                if(effect)
                {
                    clip->addFalloffEffect(effect);

                    auto effectData = dynamic_cast<ActionFalloffData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ActionModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
        else if(dynamic_cast<ActionEffectFolderData*>(parentData))
        {
            MenuFactory<ClipEffectInformation> factory;

            auto effects = photonApp->plugins()->clipEffects();
            for(auto &info : effects)
            {
                factory.addItem(info.categories, info);
            }

            ClipEffectInformation selectedInfo;
            if(factory.showMenu(event->globalPosition().toPoint(), selectedInfo))
            {

                SurfaceAction *clip = dynamic_cast<ActionEffectFolderData*>(parentData)->action();

                auto effect = photonApp->plugins()->createClipEffect(selectedInfo.id);

                if(effect)
                {
                    clip->addClipEffect(effect);

                    auto effectData = dynamic_cast<ActionEffectFolderData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ActionModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
        else if(dynamic_cast<ActionMaskData*>(parentData))
        {
            MenuFactory<MaskEffectInformation> factory;
            auto effects = photonApp->plugins()->maskEffects();
            for(auto &info : effects)
            {
                factory.addItem(info.categories, info);
            }

            MaskEffectInformation selectedInfo;
            if(factory.showMenu(event->globalPosition().toPoint(), selectedInfo))
            {

                FixtureAction *clip = dynamic_cast<ActionMaskData*>(parentData)->action();

                auto effect = photonApp->plugins()->createMaskEffect(selectedInfo.effectId);

                if(effect)
                {
                    clip->addMaskEffect(effect);

                    auto effectData = dynamic_cast<ActionMaskData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ActionModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
        /*
        else if(dynamic_cast<PixelLayoutFolderData*>(parentData))
        {
            CanvasLayerGroup *layer = dynamic_cast<PixelLayoutFolderData*>(parentData)->layer();
            QMenu menu;
            auto pixelLayouts = photonApp->project()->pixelLayouts()->layouts();
            for(auto layout : pixelLayouts)
            {
                menu.addAction(layout->name(),[layer, layout](){layer->addPixelLayout(layout);});
            }
            menu.exec(event->globalPosition().toPoint());
        }
*/

    }
    else
    {
        if(!item.isValid())
            clearSelection();
        else if(event->buttons() & Qt::RightButton)
        {
            auto itemData = static_cast<ActionModel*>(model())->dataForIndex(item);

            if(dynamic_cast<FalloffEffectData*>(itemData))
            {
                auto falloffItem = dynamic_cast<FalloffEffectData*>(itemData);

                QMenu itemMenu;
                itemMenu.addAction("Remove",[falloffItem](){
                    dynamic_cast<FixtureClip*>(falloffItem->effect()->parent())->removeFalloffEffect(falloffItem->effect());
                });

                itemMenu.exec(event->globalPosition().toPoint());
            }

            if(dynamic_cast<ChannelEffectData*>(itemData))
            {
                auto effectItem = dynamic_cast<ChannelEffectData*>(itemData);

                QMenu itemMenu;
                itemMenu.addAction("Remove",[effectItem](){
                    effectItem->effect()->channel()->removeEffect(effectItem->effect());
                });

                itemMenu.exec(event->globalPosition().toPoint());
            }

            if(dynamic_cast<BaseEffectData*>(itemData))
            {
                auto effectItem = dynamic_cast<BaseEffectData*>(itemData);

                QMenu itemMenu;
                itemMenu.addAction("Remove",[effectItem](){
                    effectItem->effect()->effectParent()->removeClipEffect(effectItem->effect());
                });

                itemMenu.exec(event->globalPosition().toPoint());
            }
            /*
            if(dynamic_cast<PixelLayoutData*>(itemData))
            {
                auto layoutItem = dynamic_cast<PixelLayoutData*>(itemData);
                auto parentItem = dynamic_cast<PixelLayoutFolderData*>(itemData->parent());

                QMenu itemMenu;
                itemMenu.addAction("Remove",[layoutItem, parentItem](){
                    parentItem->layer()->removePixelLayout(layoutItem->pixelLayout());
                });

                itemMenu.exec(event->globalPosition().toPoint());
            }
*/

        }
        QTreeView::mousePressEvent(event);
    }
}

ActionStructureViewer::ActionStructureViewer(QWidget *parent)
    : QWidget{parent}
{
    m_model = new ActionModel;
    m_treeView = new ActionTreeView;
    m_treeView->setHeaderHidden(true);
    m_treeView->setModel(m_model);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ActionStructureViewer::selectionChanged);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_treeView);

    setLayout(vLayout);
}

ActionStructureViewer::~ActionStructureViewer()
{

}

void ActionStructureViewer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    auto indexes = selected.indexes();
    if(indexes.isEmpty())
    {
        emit clearSelection();
        return;
    }
    auto itemData = m_model->dataForIndex(indexes.first());

    ChannelEffectData *effectData = dynamic_cast<ChannelEffectData*>(itemData);
    if(effectData)
    {
        //m_states.insert(m_container->uniqueId(),effectData->effect()->uniqueId());
        emit selectEffect(effectData->effect());
    }
    else if(dynamic_cast<FalloffEffectData*>(itemData))
    {
        //m_states.insert(m_action->uniqueId(),dynamic_cast<FalloffEffectData*>(itemData)->effect()->uniqueId());
        emit selectFalloff(dynamic_cast<FalloffEffectData*>(itemData)->effect());
    }
    else if(dynamic_cast<ActionStateData*>(itemData))
    {
        //m_states.insert(m_action->uniqueId(),dynamic_cast<ActionStateData*>(itemData)->state()->uniqueId());
        emit selectState(dynamic_cast<ActionStateData*>(itemData)->state());
    }
    else if(dynamic_cast<ActionParameterData*>(itemData))
    {
        //m_states.insert(m_action->uniqueId(),dynamic_cast<ActionParameterData*>(itemData)->action()->uniqueId());
        emit selectActionParameter(dynamic_cast<ActionParameterData*>(itemData)->action());
    }
    else if(dynamic_cast<MaskEffectData*>(itemData))
    {
        //m_states.insert(m_action->uniqueId(),dynamic_cast<MaskEffectData*>(itemData)->effect()->uniqueId());
        emit selectMask(dynamic_cast<MaskEffectData*>(itemData)->effect());
    }
    else if(dynamic_cast<BaseEffectData*>(itemData))
    {
        //m_states.insert(m_action->uniqueId(),dynamic_cast<BaseEffectData*>(itemData)->effect()->uniqueId());
        emit selectBaseEffect(dynamic_cast<BaseEffectData*>(itemData)->effect());
    }
    else if(dynamic_cast<ActionData*>(itemData))
    {
        emit selectAction(dynamic_cast<ActionData*>(itemData)->action());
    }
    else
    {
        //m_states.remove(m_action->uniqueId());
        emit clearSelection();
    }

}

void ActionStructureViewer::viewId(const QByteArray &t_id)
{
    auto data = m_model->root()->findDataWithId(t_id);
    if(data){
        auto index = m_model->indexForData(data);
        m_treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
    }
}

void ActionStructureViewer::restoreState()
{
    if(!m_container)
        return;
/*
    if(m_states.contains(m_action->uniqueId()))
    {
        viewId(m_states.value(m_action->uniqueId()));
        return;
    }
*/
}

void ActionStructureViewer::setContainer(SurfaceGizmoContainer *t_container)
{
    m_container = t_container;

    if(m_container)
    {
        m_model->setContainer(t_container);
        m_treeView->expandRecursively(m_model->index(0,0));
    }
    else
    {
        emit clearSelection();
    }

}


} // namespace photon


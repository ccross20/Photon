#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include "clipstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/fixtureclip.h"
#include "falloff/falloffeffect.h"
#include "fixture/maskeffect.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "gui/menufactory.h"
#include "state/state.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "sequence/canvasclip.h"
#include "sequence/canvaslayergroup.h"

namespace photon {

ClipTreeView::ClipTreeView() : QTreeView()
{

}

void ClipTreeView::mousePressEvent(QMouseEvent *event)
{
    auto item = indexAt(event->pos());
    auto itemData = static_cast<ClipModel*>(model())->dataForIndex(item);
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
                    auto effectIndex = static_cast<ClipModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }

        }
        else if(dynamic_cast<ClipFalloffData*>(parentData))
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

                FixtureClip *clip = dynamic_cast<ClipFalloffData*>(parentData)->clip();

                auto effect = photonApp->plugins()->createFalloffEffect(selectedInfo.effectId);

                if(effect)
                {
                    clip->addFalloffEffect(effect);

                    auto effectData = dynamic_cast<ClipFalloffData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ClipModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
        else if(dynamic_cast<ClipEffectFolderData*>(parentData))
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

                Clip *clip = dynamic_cast<ClipEffectFolderData*>(parentData)->clip();

                auto effect = photonApp->plugins()->createClipEffect(selectedInfo.id);

                if(effect)
                {
                    clip->addClipEffect(effect);

                    auto effectData = dynamic_cast<ClipEffectFolderData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ClipModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
        else if(dynamic_cast<ClipMaskData*>(parentData))
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

                FixtureClip *clip = dynamic_cast<ClipMaskData*>(parentData)->clip();

                auto effect = photonApp->plugins()->createMaskEffect(selectedInfo.effectId);

                if(effect)
                {
                    clip->addMaskEffect(effect);

                    auto effectData = dynamic_cast<ClipMaskData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ClipModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
        }
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

    }
    else
    {
        if(!item.isValid())
            clearSelection();
        else if(event->buttons() & Qt::RightButton)
        {
            auto itemData = static_cast<ClipModel*>(model())->dataForIndex(item);

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

        }
        QTreeView::mousePressEvent(event);
    }
}

ClipStructureViewer::ClipStructureViewer(QWidget *parent)
    : QWidget{parent}
{
    m_model = new ClipModel;
    m_treeView = new ClipTreeView;
    m_treeView->setHeaderHidden(true);
    m_treeView->setModel(m_model);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ClipStructureViewer::selectionChanged);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_treeView);

    setLayout(vLayout);
}

ClipStructureViewer::~ClipStructureViewer()
{

}

void ClipStructureViewer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
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
        m_states.insert(m_clip->uniqueId(),effectData->effect()->uniqueId());
        emit selectEffect(effectData->effect());
    }
    else if(dynamic_cast<FalloffEffectData*>(itemData))
    {
        m_states.insert(m_clip->uniqueId(),dynamic_cast<FalloffEffectData*>(itemData)->effect()->uniqueId());
        emit selectFalloff(dynamic_cast<FalloffEffectData*>(itemData)->effect());
    }
    else if(dynamic_cast<ClipStateData*>(itemData))
    {
        m_states.insert(m_clip->uniqueId(),dynamic_cast<ClipStateData*>(itemData)->state()->uniqueId());
        emit selectState(dynamic_cast<ClipStateData*>(itemData)->state());
    }
    else if(dynamic_cast<ClipParameterData*>(itemData))
    {
        m_states.insert(m_clip->uniqueId(),dynamic_cast<ClipParameterData*>(itemData)->clip()->uniqueId());
        emit selectClipParameter(dynamic_cast<ClipParameterData*>(itemData)->clip());
    }
    else if(dynamic_cast<MaskEffectData*>(itemData))
    {
        m_states.insert(m_clip->uniqueId(),dynamic_cast<MaskEffectData*>(itemData)->effect()->uniqueId());
        emit selectMask(dynamic_cast<MaskEffectData*>(itemData)->effect());
    }
    else if(dynamic_cast<BaseEffectData*>(itemData))
    {
        m_states.insert(m_clip->uniqueId(),dynamic_cast<BaseEffectData*>(itemData)->effect()->uniqueId());
        emit selectClipEffect(dynamic_cast<BaseEffectData*>(itemData)->effect());
    }
    else
    {
        m_states.remove(m_clip->uniqueId());
        emit clearSelection();
    }

}

void ClipStructureViewer::viewId(const QByteArray &t_id)
{
    auto data = m_model->root()->findDataWithId(t_id);
    if(data){
        auto index = m_model->indexForData(data);
        m_treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
    }
}

void ClipStructureViewer::restoreState()
{
    if(!m_clip)
        return;

    if(m_states.contains(m_clip->uniqueId()))
    {
        viewId(m_states.value(m_clip->uniqueId()));
        return;
    }
}

void ClipStructureViewer::setClip(Clip *t_clip)
{
    if(t_clip == m_clip)
        return;

    if(m_clip)
        m_model->removeClip(m_clip);

    m_clip = t_clip;

    if(m_clip)
    {
        m_model->addClip(m_clip);
        m_treeView->expandAll();
    }
    else
    {
        emit clearSelection();
    }

}


void ClipStructureViewer::addMasterLayer(MasterLayer *t_layer)
{
    m_model->addMasterLayer(t_layer);
    m_treeView->expandAll();
}

void ClipStructureViewer::removeMasterLayer(MasterLayer *t_layer)
{
    m_model->removeMasterLayer(t_layer);
    emit clearSelection();
}

} // namespace photon

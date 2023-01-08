#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include "clipstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"
#include "falloff/falloffeffect.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "gui/menufactory.h"

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
        else if(dynamic_cast<FalloffData*>(parentData))
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

                Clip *clip = dynamic_cast<FalloffData*>(parentData)->clip();

                auto effect = photonApp->plugins()->createFalloffEffect(selectedInfo.effectId);

                if(effect)
                {
                    clip->addFalloffEffect(effect);

                    auto effectData = dynamic_cast<FalloffData*>(parentData)->findEffectData(effect);
                    auto effectIndex = static_cast<ClipModel*>(model())->indexForData(effectData);

                    if(effectIndex.isValid())
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                }
            }
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
                    falloffItem->effect()->clip()->removeFalloffEffect(falloffItem->effect());
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
        emit selectEffect(effectData->effect());
    }
    else if(dynamic_cast<FalloffEffectData*>(itemData))
    {
        emit selectFalloff(dynamic_cast<FalloffEffectData*>(itemData)->effect());
    }
    else
    {
        emit clearSelection();
    }

}

void ClipStructureViewer::addClip(Clip *t_clip)
{
    m_model->addClip(t_clip);
    m_treeView->expandAll();
}

void ClipStructureViewer::removeClip(Clip *t_clip)
{
    m_model->removeClip(t_clip);
    emit clearSelection();
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

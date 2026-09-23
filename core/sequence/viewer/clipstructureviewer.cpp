#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>
#include "clipstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/fixtureclip.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "gui/menufactory.h"
#include "state/state.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"

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
                    {
                        selectionModel()->select(effectIndex, QItemSelectionModel::ClearAndSelect);
                        setCurrentIndex(effectIndex);
                        reassertSelection(QPersistentModelIndex(effectIndex));
                    }
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

void ClipTreeView::reassertSelection(const QPersistentModelIndex &index)
{
    QTimer::singleShot(0, this, [this, index](){
        if(index.isValid())
        {
            selectionModel()->select(QModelIndex(index), QItemSelectionModel::ClearAndSelect);
            setCurrentIndex(QModelIndex(index));
        }
    });
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
    else if(dynamic_cast<ClipGraphData*>(itemData))
    {
        m_states.insert(m_clip->uniqueId(),dynamic_cast<ClipGraphData*>(itemData)->clip()->uniqueId());
        emit selectClipGraph(dynamic_cast<ClipGraphData*>(itemData)->clip());
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


} // namespace photon

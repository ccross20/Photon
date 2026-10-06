#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>
#include <QPointer>
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

    if(auto *createData = dynamic_cast<CreateData*>(itemData))
    {
        if(auto *channelData = dynamic_cast<ChannelData*>(createData->parent()))
        {
            QPointer<Channel> channel = channelData->channel();
            const QPoint globalPos = event->globalPosition().toPoint();
            QTimer::singleShot(0, this, [this, channel, globalPos](){
                if(channel)
                    showAddEffectMenu(channel, globalPos);
            });
        }
        event->accept();
        return;
    }

    if(!item.isValid())
    {
        clearSelection();
        QTreeView::mousePressEvent(event);
        return;
    }

    if(event->button() == Qt::RightButton)
    {
        if(auto *effectItem = dynamic_cast<ChannelEffectData*>(itemData))
        {
            ChannelEffect *effect = effectItem->effect();
            QMenu itemMenu;
            itemMenu.addAction("Remove", [effect](){
                effect->channel()->removeEffect(effect);
            });
            itemMenu.exec(event->globalPosition().toPoint());
            // Not handed on to the tree: the row under the cursor may now be
            // a different one, and the tree would select it.
            event->accept();
            return;
        }
    }

    QTreeView::mousePressEvent(event);
}

void ClipTreeView::showAddEffectMenu(Channel *t_channel, const QPoint &t_globalPos)
{
    MenuFactory<EffectInformation> factory;
    for(auto &info : photonApp->plugins()->channelEffects())
        factory.addItem(info.categories, info);

    EffectInformation selectedInfo;
    if(!factory.showMenu(t_globalPos, selectedInfo))
        return;

    auto effect = photonApp->plugins()->createChannelEffect(selectedInfo.effectId);
    if(!effect)
        return;

    t_channel->addEffect(effect);
    emit effectCreated(effect);
}

ClipStructureViewer::ClipStructureViewer(QWidget *parent)
    : QWidget{parent}
{
    m_model = new ClipModel;
    m_treeView = new ClipTreeView;
    m_treeView->setHeaderHidden(true);
    m_treeView->setModel(m_model);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ClipStructureViewer::selectionChanged);
    connect(m_treeView, &ClipTreeView::effectCreated, this, &ClipStructureViewer::selectEffectRow);
    // Qt doesn't report a selection change when the selected row itself is
    // removed, so watch for that separately.
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &ClipStructureViewer::rowsRemoved);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_treeView);

    setLayout(vLayout);
}

ClipStructureViewer::~ClipStructureViewer()
{

}

void ClipStructureViewer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Only a choice the user makes changes what's remembered for the clip -
    // not the selection shifting while a clip is swapped in or its view is
    // being restored (see setClip() / restoreState()).
    const bool remember = m_clip && !m_switching;

    auto indexes = selected.indexes();
    if(indexes.isEmpty())
    {
        m_hasEditorSelection = false;
        emit clearSelection();
        return;
    }
    auto itemData = m_model->dataForIndex(indexes.first());

    // Remembered by the row's own id ("graph" for the clip graph row), which
    // is what viewId() looks up. The clip's uniqueId would find the clip's
    // own top-level row instead.
    if(auto *effectData = dynamic_cast<ChannelEffectData*>(itemData))
    {
        m_hasEditorSelection = true;
        if(remember)
            m_states.insert(m_clip->uniqueId(), effectData->id());
        emit selectEffect(effectData->effect());
    }
    else if(auto *graphData = dynamic_cast<ClipGraphData*>(itemData))
    {
        m_hasEditorSelection = true;
        if(remember)
            m_states.insert(m_clip->uniqueId(), graphData->id());
        emit selectClipGraph(graphData->clip());
    }
    else
    {
        // The clip's own (top-level) row shows its properties; the folder
        // rows below it just show the default editor. Either is remembered
        // like any other view.
        m_hasEditorSelection = false;
        if(remember)
            m_states.insert(m_clip->uniqueId(), itemData->id());
        emit clearSelection();
        if(auto *clipData = dynamic_cast<ClipData*>(itemData))
            emit selectClipProperties(clipData->clip());
    }

}

void ClipStructureViewer::selectEffectRow(ChannelEffect *t_effect)
{
    const QModelIndex index = m_model->indexForId(t_effect->uniqueId());
    if(!index.isValid())
        return;
    m_treeView->scrollTo(index);
    m_treeView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    m_treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
}

void ClipStructureViewer::rowsRemoved()
{
    if(!m_hasEditorSelection || m_treeView->selectionModel()->hasSelection())
        return;

    // The selected effect (or its channel/clip) was removed.
    m_hasEditorSelection = false;
    if(m_clip && !m_switching)
        m_states.remove(m_clip->uniqueId());
    emit clearSelection();
}

void ClipStructureViewer::viewId(const QByteArray &t_id)
{
    auto data = m_model->root()->findDataWithId(t_id);
    if(data){
        auto index = m_model->indexForData(data);
        m_treeView->scrollTo(index);
        m_treeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
        m_treeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    }
}

void ClipStructureViewer::restoreState()
{
    if(!m_clip)
        return;

    m_switching = true;
    QByteArray remembered = m_states.value(m_clip->uniqueId());
    // Nothing remembered (or what was is gone): the clip's own row, so a
    // clip opens on its properties.
    if(remembered.isEmpty() || !m_model->indexForId(remembered).isValid())
        remembered = m_clip->uniqueId();

    // Cleared first so the selection always changes - reselecting a row
    // that was already selected wouldn't re-show its view.
    m_treeView->selectionModel()->clearSelection();
    viewId(remembered);
    m_switching = false;
}

void ClipStructureViewer::setClip(Clip *t_clip)
{
    if(t_clip == m_clip)
        return;

    // The selection shifts on its own as the old clip's rows go and the new
    // one's arrive; m_switching keeps that from rewriting either clip's
    // remembered view.
    m_switching = true;
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
    m_switching = false;

}


} // namespace photon

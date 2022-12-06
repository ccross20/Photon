#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QSplitter>
#include <QScrollBar>
#include <QShowEvent>
#include <QToolBar>
#include <QStackedWidget>
#include "sequencewidget.h"
#include "clipeffecteditor.h"
#include "timelineviewer.h"
#include "timelinescene.h"
#include "sequenceclip.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "timekeeper.h"
#include "timelineheader.h"
#include "clipstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/clip.h"
#include "falloff/falloffeffect.h"
#include "timebar.h"

namespace photon {

class SequenceWidget::Impl
{
public:
    QSplitter *horizontalSplitter;
    QSplitter *verticalSplitter;
    QSplitter *detailsSplitter;
    QSplitter *timeSplitter;
    TimelineViewer *viewer = nullptr;
    TimelineHeader *details;
    TimeBar *timebar;
    QToolBar *timeToolBar;
    ClipStructureViewer *curvePropertyEditor;
    QWidget *effectEditorContainer;
    QWidget *effectEditor = nullptr;
    TimelineScene *scene;
    QElapsedTimer timer;
    QVector<SequenceClip*> selectedClips;
    double currentTime = 0;
    double offset = 0;
    double scale = 20.0;
    bool isPlaying = false;
};

SequenceWidget::SequenceWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->horizontalSplitter = new QSplitter;
    m_impl->verticalSplitter = new QSplitter(Qt::Vertical);
    m_impl->detailsSplitter = new QSplitter(Qt::Vertical);
    m_impl->timeSplitter = new QSplitter(Qt::Horizontal);

    m_impl->timebar = new TimeBar;
    m_impl->timebar->setScale(m_impl->scale);
    m_impl->timeToolBar = new QToolBar;
    m_impl->scene = new TimelineScene;
    m_impl->viewer = new TimelineViewer;
    //m_impl->viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_impl->viewer->setScene(m_impl->scene);
    m_impl->viewer->setScale(m_impl->scale);

    m_impl->details = new TimelineHeader;
    m_impl->curvePropertyEditor = new ClipStructureViewer;

    m_impl->effectEditorContainer = new QWidget;

    m_impl->timeSplitter->addWidget(m_impl->timeToolBar);
    m_impl->timeSplitter->addWidget(m_impl->timebar);



    connect(m_impl->timeToolBar->addAction("Rewind"), &QAction::triggered, this, &SequenceWidget::rewind);
    auto playAction = m_impl->timeToolBar->addAction("Play");
    playAction->setCheckable(true);
    connect(playAction, &QAction::toggled, this, &SequenceWidget::togglePlay);



    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->timeSplitter);
    vLayout->addWidget(m_impl->horizontalSplitter);

    //m_impl->scene->setSceneRect(0,0,300,100);

    m_impl->verticalSplitter->addWidget(m_impl->viewer);
    m_impl->verticalSplitter->addWidget(m_impl->effectEditorContainer);

    m_impl->viewer->centerOn(0,0);

    m_impl->detailsSplitter->addWidget(m_impl->details);
    m_impl->detailsSplitter->addWidget(m_impl->curvePropertyEditor);

    m_impl->horizontalSplitter->addWidget(m_impl->detailsSplitter);
    m_impl->horizontalSplitter->addWidget(m_impl->verticalSplitter);
    setLayout(vLayout);
    connect(photonApp->timekeeper(), &Timekeeper::tick, this, &SequenceWidget::tick);
    connect(m_impl->viewer->verticalScrollBar(), &QAbstractSlider::valueChanged, m_impl->details, &TimelineHeader::offsetChanged);
    connect(m_impl->viewer, &TimelineViewer::scaleChanged, this, &SequenceWidget::setScale);

    connect(m_impl->scene, &TimelineScene::selectionChanged, this, &SequenceWidget::selectionChanged);
    connect(m_impl->verticalSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::editorSplitterMoved);
    connect(m_impl->detailsSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::detailsSplitterMoved);
    connect(m_impl->horizontalSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::horizontalSplitterMoved);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectFalloff, this, &SequenceWidget::selectFalloff);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectEffect, this, &SequenceWidget::selectEffect);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::clearSelection, this, &SequenceWidget::clearEditor);
    connect(m_impl->timebar, &TimeBar::changeTime, this, &SequenceWidget::gotoTime);
    connect(m_impl->viewer->horizontalScrollBar(), &QAbstractSlider::valueChanged, m_impl->timebar, &TimeBar::setOffset);
    connect(m_impl->viewer->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &SequenceWidget::xOffsetChanged);

}

SequenceWidget::~SequenceWidget()
{
    delete m_impl;
}

void SequenceWidget::setSequence(Sequence *t_sequence)
{
    m_impl->scene->setSequence(t_sequence);
    m_impl->viewer->centerOn(0,0);
    m_impl->details->setSequence(t_sequence);
}

Sequence *SequenceWidget::sequence() const
{
    return m_impl->scene->sequence();
}

void SequenceWidget::setScale(double t_scale)
{
    m_impl->scale = t_scale;
    m_impl->timebar->setScale(t_scale);
    m_impl->viewer->setScale(t_scale);
}

void SequenceWidget::selectEffect(photon::ChannelEffect *t_effect)
{

    clearEditor();

    auto editor = t_effect->createEditor();
    editor->setOffset(m_impl->offset);
    editor->setScale(m_impl->scale);
    connect(m_impl->viewer->horizontalScrollBar(), &QAbstractSlider::valueChanged, editor, &ChannelEffectEditor::setOffset);
    connect(m_impl->viewer, &TimelineViewer::scaleChanged, editor, &ChannelEffectEditor::setScale);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(editor);

    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);


    //editor->selectEffect(t_effect);
}

void SequenceWidget::selectFalloff(photon::FalloffEffect *t_effect)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = t_effect->createEditor();
    layout->addWidget(editor);
    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);

}

void SequenceWidget::clearEditor()
{
    if(m_impl->effectEditorContainer->layout())
        delete m_impl->effectEditorContainer->layout();
    if(m_impl->effectEditor)
        delete m_impl->effectEditor;

    m_impl->effectEditor = nullptr;
}

void SequenceWidget::selectionChanged()
{
    auto newSelection = m_impl->scene->selectedItems();
    QVector<SequenceClip*> removed;
    QVector<SequenceClip*> added;

    for(auto item : m_impl->selectedClips)
    {
        if(!newSelection.contains(item))
        {
            SequenceClip *clip = dynamic_cast<SequenceClip*>(item);
            removed.append(item);
            if(clip)
                m_impl->curvePropertyEditor->removeClip(clip->clip());
        }
    }

    for(auto item : newSelection)
    {
        SequenceClip *clip = dynamic_cast<SequenceClip*>(item);
        if(clip)
        {
            if(!m_impl->selectedClips.contains(clip))
            {
                added.append(clip);
                m_impl->curvePropertyEditor->addClip(clip->clip());
            }
        }
    }

    for(auto clip : removed)
        m_impl->selectedClips.removeOne(clip);

    m_impl->selectedClips.append(added);
}

void SequenceWidget::xOffsetChanged(int t_value)
{
    m_impl->offset = t_value;
}

void SequenceWidget::gotoTime(double t_time)
{
    m_impl->currentTime = t_time;
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
}

void SequenceWidget::tick()
{
    if(!m_impl->isPlaying)
        return;

    //m_impl->currentTime += m_impl->timer.nsecsElapsed() * 1000000.0;
    m_impl->currentTime += m_impl->timer.elapsed()/1000.0;
    m_impl->timer.restart();
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
}

void SequenceWidget::detailsSplitterMoved(int pos, int index)
{
    m_impl->verticalSplitter->setSizes(m_impl->detailsSplitter->sizes());
}

void SequenceWidget::editorSplitterMoved(int pos, int index)
{
    m_impl->detailsSplitter->setSizes(m_impl->verticalSplitter->sizes());
}

void SequenceWidget::horizontalSplitterMoved(int pos, int index)
{
    m_impl->timeSplitter->setSizes(m_impl->horizontalSplitter->sizes());
}

void SequenceWidget::processPreview(ProcessContext &context)
{
    if(!m_impl->isPlaying)
        return;
    context.globalTime = m_impl->currentTime;
    m_impl->scene->sequence()->processChannels(context, 0);
}

void SequenceWidget::togglePlay(bool t_value)
{
    m_impl->timer.restart();
    m_impl->isPlaying = t_value;
}

void SequenceWidget::rewind()
{
    m_impl->currentTime = 0.0;
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
}

void SequenceWidget::showEvent(QShowEvent*t_event)
{
    QWidget::showEvent(t_event);

    int halfHeight = height()/2;
    m_impl->horizontalSplitter->setSizes({static_cast<int>(width()*.2),static_cast<int>(width()*.8)});
    m_impl->verticalSplitter->setSizes({halfHeight,halfHeight});
    m_impl->detailsSplitter->setSizes({halfHeight,halfHeight});
    m_impl->timeSplitter->setSizes(m_impl->horizontalSplitter->sizes());
}


} // namespace photon

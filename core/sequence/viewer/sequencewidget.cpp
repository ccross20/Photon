#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QSplitter>
#include <QScrollBar>
#include <QShowEvent>
#include <QResizeEvent>
#include <QToolBar>
#include <QFileDialog>
#include <QStackedWidget>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QAudioDevice>
#include <QAudioOutput>
#include "sequencewidget.h"
#include "timelineviewer.h"
#include "timelinescene.h"
#include "sequenceclip.h"
#include "waveformheader.h"
#include "timelinemasterlayer.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "timekeeper.h"
#include "timelineheader.h"
#include "clipstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/clip.h"
#include "state/state.h"
#include "sequence/masterlayer.h"
#include "falloff/falloffeffect.h"
#include "timebar.h"
#include "graph/bus/busevaluator.h"
#include "gui/waveformwidget.h"
#include "sequence/viewer/stateeditor.h"
#include "sequence/fixtureclip.h"
#include "sequence/clipeffect.h"
#include "fixture/maskeffect.h"
#include "sequencewaveformeditor.h"

namespace photon {

class SequenceWidget::Impl
{
public:
    double visibleStartTime() const;
    double visibleEndTime() const;

    QSplitter *horizontalSplitter;
    QSplitter *verticalSplitter;
    QSplitter *detailsSplitter;
    QSplitter *timeSplitter;
    TimelineViewer *viewer = nullptr;
    TimelineHeader *details;
    WaveformHeader *waveformHeader;
    TimeBar *timebar;
    QToolBar *timeToolBar;
    ClipStructureViewer *curvePropertyEditor;
    QWidget *effectEditorContainer;
    QWidget *effectEditor = nullptr;
    SequenceWaveformEditor *waveform = nullptr;
    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;
    TimelineScene *scene;
    QElapsedTimer timer;
    QVector<SequenceClip*> selectedClips;
    QVector<TimelineMasterLayer*> selectedLayers;
    qint64 startTimeMS;
    double lastCurrentTime = 0;
    double currentTime = 0;
    double lastPreviewTime = -1;
    double offset = 0;
    double scale = 20.0;
    bool isPlaying = false;
};

double SequenceWidget::Impl::visibleStartTime() const
{
    return offset / scale;
}

double SequenceWidget::Impl::visibleEndTime() const
{
    return (offset + waveform->width()) / scale;
}

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
    m_impl->waveform = new SequenceWaveformEditor;
    m_impl->waveformHeader = new WaveformHeader;
    //m_impl->viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_impl->viewer->setScene(m_impl->scene);
    m_impl->viewer->setScale(m_impl->scale);

    m_impl->details = new TimelineHeader;
    m_impl->curvePropertyEditor = new ClipStructureViewer;

    m_impl->effectEditorContainer = new QWidget;

    m_impl->timeSplitter->addWidget(m_impl->timeToolBar);
    m_impl->timeSplitter->addWidget(m_impl->timebar);



    m_impl->player = new QMediaPlayer(this);
    m_impl->audioOutput = new QAudioOutput(this);
    m_impl->player->setAudioOutput(m_impl->audioOutput);


    connect(m_impl->timeToolBar->addAction("Rewind"), &QAction::triggered, this, &SequenceWidget::rewind);
    auto playAction = m_impl->timeToolBar->addAction("Play");
    playAction->setShortcut(Qt::Key_Space);
    playAction->setCheckable(true);
    connect(playAction, &QAction::toggled, this, &SequenceWidget::togglePlay);
    connect(m_impl->timeToolBar->addAction("Load"), &QAction::triggered, this, &SequenceWidget::pickFile);



    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->timeSplitter);
    vLayout->addWidget(m_impl->horizontalSplitter);

    //m_impl->scene->setSceneRect(0,0,300,100);

    m_impl->verticalSplitter->addWidget(m_impl->viewer);
    m_impl->verticalSplitter->addWidget(m_impl->waveform);
    m_impl->verticalSplitter->addWidget(m_impl->effectEditorContainer);

    //m_impl->viewer->centerOn(0,0);

    m_impl->detailsSplitter->addWidget(m_impl->details);
    m_impl->detailsSplitter->addWidget(m_impl->waveformHeader);
    m_impl->detailsSplitter->addWidget(m_impl->curvePropertyEditor);

    m_impl->horizontalSplitter->addWidget(m_impl->detailsSplitter);
    m_impl->horizontalSplitter->addWidget(m_impl->verticalSplitter);
    setLayout(vLayout);
    connect(photonApp->timekeeper(), &Timekeeper::tick, this, &SequenceWidget::tick);
    //connect(m_impl->viewer->verticalScrollBar(), &QAbstractSlider::valueChanged, m_impl->details, &TimelineHeader::offsetChanged);
    connect(m_impl->viewer, &TimelineViewer::scaleChanged, this, &SequenceWidget::setScale);

    connect(m_impl->scene, &TimelineScene::selectionChanged, this, &SequenceWidget::selectionChanged);
    connect(m_impl->verticalSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::editorSplitterMoved);
    connect(m_impl->detailsSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::detailsSplitterMoved);
    connect(m_impl->horizontalSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::horizontalSplitterMoved);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectFalloff, this, &SequenceWidget::selectFalloff);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectState, this, &SequenceWidget::selectState);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectEffect, this, &SequenceWidget::selectEffect);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectMask, this, &SequenceWidget::selectMask);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectClipEffect, this, &SequenceWidget::selectClipEffect);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::clearSelection, this, &SequenceWidget::clearEditor);
    connect(m_impl->timebar, &TimeBar::changeTime, this, &SequenceWidget::gotoTime);
    connect(m_impl->viewer, &TimelineViewer::offsetChanged, this, &SequenceWidget::setOffset);

    connect(m_impl->player, &QMediaPlayer::positionChanged, this, &SequenceWidget::positionChanged);

}

SequenceWidget::~SequenceWidget()
{
    delete m_impl;
}

void SequenceWidget::setSequence(Sequence *t_sequence)
{
    m_impl->scene->setSequence(t_sequence);
    //m_impl->viewer->centerOn(0,0);
    m_impl->details->setSequence(t_sequence);
    m_impl->player->setSource(t_sequence->filePath());
    m_impl->waveform->setSequence(t_sequence);
    m_impl->waveformHeader->setSequence(t_sequence);
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

    ChannelEffectEditor *channelEditor = dynamic_cast<ChannelEffectEditor*>(m_impl->effectEditor);
    if(channelEditor)
        channelEditor->setXScale(t_scale);
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

void SequenceWidget::setOffset(double t_offset)
{
    m_impl->offset = t_offset;
    m_impl->timebar->setOffset(t_offset);
    m_impl->viewer->setOffset(t_offset);

    ChannelEffectEditor *channelEditor = dynamic_cast<ChannelEffectEditor*>(m_impl->effectEditor);
    if(channelEditor)
        channelEditor->setOffset(t_offset);
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

void SequenceWidget::selectEffect(photon::ChannelEffect *t_effect)
{

    clearEditor();

    auto editor = t_effect->createEditor();
    editor->setOffset(m_impl->offset);
    editor->setScale(QPointF(m_impl->scale,editor->scale().y()));
    connect(editor, &ChannelEffectEditor::offsetChanged, this, &SequenceWidget::setOffset);
    connect(editor, &ChannelEffectEditor::xScaleChanged, m_impl->viewer, &TimelineViewer::setScale);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(editor);

    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);


    //editor->selectEffect(t_effect);
}

void SequenceWidget::selectMask(photon::MaskEffect *t_effect)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = t_effect->createEditor();
    layout->addWidget(editor);
    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);
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

void SequenceWidget::selectClipEffect(photon::ClipEffect *t_effect)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = t_effect->createEditor();

    if(editor)
    {
        layout->addWidget(editor);
        m_impl->effectEditor = editor;
        m_impl->effectEditorContainer->setLayout(layout);
    }


}

void SequenceWidget::selectState(photon::State *t_state)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = new StateEditor;

    if(m_impl->selectedClips.length() > 0)
    {
        auto clip = dynamic_cast<FixtureClip *>(m_impl->selectedClips[0]->clip());
        if(clip)
            editor->setClip(clip);
    }

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
    QVector<TimelineMasterLayer*> removedLayers;
    QVector<TimelineMasterLayer*> addedLayers;

    for(auto item : m_impl->selectedClips)
    {
        if(!newSelection.contains(item))
        {
            SequenceClip *clip = dynamic_cast<SequenceClip*>(item);
            removed.append(item);
            if(clip)
                m_impl->curvePropertyEditor->setClip(nullptr);
        }
    }
    for(auto item : m_impl->selectedLayers)
    {
        if(!newSelection.contains(item))
        {
            auto layer = dynamic_cast<TimelineMasterLayer*>(item);
            removedLayers.append(item);
            if(layer)
                m_impl->curvePropertyEditor->removeMasterLayer(static_cast<MasterLayer*>(layer->layer()));
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
                m_impl->curvePropertyEditor->setClip(clip->clip());
            }
        }

        auto layer = dynamic_cast<TimelineMasterLayer*>(item);
        if(layer)
        {
            if(!m_impl->selectedLayers.contains(layer))
            {
                addedLayers.append(layer);
                m_impl->curvePropertyEditor->addMasterLayer(static_cast<MasterLayer*>(layer->layer()));
            }
        }
    }

    for(auto clip : removed)
        m_impl->selectedClips.removeOne(clip);
    for(auto layer : removedLayers)
        m_impl->selectedLayers.removeOne(layer);

    m_impl->selectedClips.append(added);
    m_impl->selectedLayers.append(addedLayers);

    m_impl->curvePropertyEditor->restoreState();
}

void SequenceWidget::gotoTime(double t_time)
{
    m_impl->currentTime = t_time;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();
    m_impl->lastCurrentTime = t_time;
    m_impl->timer.restart();
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);

    photonApp->busEvaluator()->evaluate();
    m_impl->player->setPosition(m_impl->currentTime*1000);
    m_impl->waveform->setPlayhead(m_impl->currentTime);
}

void SequenceWidget::positionChanged(qint64 t_time)
{
    /*
    m_impl->currentTime = t_time/1000.0;
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);

    photonApp->busEvaluator()->evaluate();
    */
    //m_impl->waveform->setPlayhead(t_time/1000.0);
}

void SequenceWidget::tick()
{
    if(!m_impl->isPlaying)
        return;
/*
    if(m_impl->player->playbackState() == QMediaPlayer::PlayingState)
        return;
        */
    //m_impl->currentTime += m_impl->timer.nsecsElapsed() * 1000000.0;

    qint64 deltaTime = QDateTime::currentMSecsSinceEpoch() - m_impl->startTimeMS;

    m_impl->currentTime = m_impl->lastCurrentTime + (deltaTime / 1000.0);
    m_impl->timer.restart();
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
    m_impl->waveform->setPlayhead(m_impl->currentTime);
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
    //if(abs(m_impl->currentTime - m_impl->lastPreviewTime) < .005)
        //return;
    context.globalTime = m_impl->currentTime;
    context.project = sequence()->project();
    m_impl->scene->sequence()->processChannels(context, 0);
    m_impl->lastPreviewTime = m_impl->currentTime;
}

DMXMatrix SequenceWidget::getDMX()
{
    qint64 deltaTime = QDateTime::currentMSecsSinceEpoch() - m_impl->startTimeMS;

    m_impl->currentTime = m_impl->lastCurrentTime + (deltaTime / 1000.0);
    m_impl->timer.restart();

    DMXMatrix matrix;
    ProcessContext context{matrix};

    context.project = sequence()->project();
    context.globalTime = m_impl->currentTime;
    m_impl->scene->sequence()->processChannels(context, 0);

    return matrix;
}

bool SequenceWidget::isPlaying() const
{
    return m_impl->isPlaying;
}

void SequenceWidget::togglePlay(bool t_value)
{
    m_impl->timer.restart();
    m_impl->isPlaying = t_value;

    m_impl->lastCurrentTime = m_impl->currentTime;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();

    if(m_impl->isPlaying)
        m_impl->player->play();
    else
        m_impl->player->pause();
}

void SequenceWidget::rewind()
{
    m_impl->currentTime = 0.0;
    m_impl->lastCurrentTime = 0.0;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
    m_impl->player->setPosition(0);
}

void SequenceWidget::pickFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    m_impl->scene->sequence()->filePath(),
                                                    "Audio Files (*.mp3 *.wav)");

    if(!fileName.isEmpty())
    {
        m_impl->scene->sequence()->setAudioPath(fileName);
        m_impl->player->setSource(fileName);
        m_impl->waveform->loadAudio(fileName);
    }

}

void SequenceWidget::showEvent(QShowEvent*t_event)
{
    QWidget::showEvent(t_event);

    int waveHeight = 100;
    int halfHeight = (height() - waveHeight) / 2;
    m_impl->horizontalSplitter->setSizes({static_cast<int>(width()*.2),static_cast<int>(width()*.8)});
    m_impl->verticalSplitter->setSizes({halfHeight,waveHeight,halfHeight});
    m_impl->detailsSplitter->setSizes({halfHeight,waveHeight,halfHeight});
    m_impl->timeSplitter->setSizes(m_impl->horizontalSplitter->sizes());
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

void SequenceWidget::resizeEvent(QResizeEvent* t_event)
{
    QWidget::resizeEvent(t_event);
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

} // namespace photon

#include <algorithm>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QSplitter>
#include <QScrollBar>
#include <QShowEvent>
#include <QResizeEvent>
#include <QToolBar>
#include <QStyle>
#include <QPainter>
#include <QPixmap>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTabWidget>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QAudioDevice>
#include <QAudioOutput>
#include "sequencewidget.h"
#include "gui/properties/propertycontroller.h"
#include "gui/properties/propertysubject.h"
#include "model/node.h"
#include "timelineviewer.h"
#include "timelinescene.h"
#include "sequenceclip.h"
#include "waveformheader.h"
#include "sequence/sequence.h"
#include "sequence/cliplayer.h"
#include "photoncore.h"
#include "timekeeper.h"
#include "graph/node/library/savedresourcedrop.h"
#include "timelineheader.h"
#include "clipstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/clip.h"
#include "sequence/layer.h"
#include "timebar.h"
#include "graph/bus/busevaluator.h"
#include "gui/waveformwidget.h"
#include "sequencewaveformeditor.h"
#include "plugin/pluginfactory.h"
#include "routine/routine.h"
#include "view/graphwidget.h"
#include "view/nodeitem.h"
#include "view/scene.h"
#include "model/graph.h"
#include "model/subgraphnode.h"
#include <QPointer>
#include "view/scene.h"
#include "virtualdj/virtualdjconnector.h"

namespace {

// The toolbar's light foreground colour - the stock style icons are drawn
// dark, which nearly vanishes on the toolbar's dark background.
const QColor kToolbarIconColor(220, 220, 220);

// A standard style icon recoloured to kToolbarIconColor, keeping its shape
// (alpha), at both 1x and 2x so it stays crisp on Retina screens.
QIcon lightIcon(const QStyle *t_style, QStyle::StandardPixmap t_pixmap)
{
    const QIcon source = t_style->standardIcon(t_pixmap);
    QIcon result;
    for(const qreal ratio : {1.0, 2.0})
    {
        QPixmap pixmap = source.pixmap(QSize(20, 20), ratio);
        if(pixmap.isNull())
            continue;
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(QRect(QPoint(0, 0), pixmap.deviceIndependentSize().toSize()), kToolbarIconColor);
        painter.end();
        result.addPixmap(pixmap);
    }
    return result;
}

// "Zoom to Fit" has no matching QStyle::StandardPixmap, unlike the other
// transport actions - drawn by hand as four corner brackets (the common
// "fit to view" glyph) rather than reaching for a real icon set that doesn't
// exist in this app yet (see projecticons.cpp for the same tradeoff).
QIcon zoomToFitIcon()
{
    const int size = 20;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(kToolbarIconColor);
    pen.setWidth(2);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    const int inset = 3;
    const int arm = 5;

    painter.drawLine(inset, inset + arm, inset, inset);
    painter.drawLine(inset, inset, inset + arm, inset);

    painter.drawLine(size - inset - arm, inset, size - inset, inset);
    painter.drawLine(size - inset, inset, size - inset, inset + arm);

    painter.drawLine(inset, size - inset - arm, inset, size - inset);
    painter.drawLine(inset, size - inset, inset + arm, size - inset);

    painter.drawLine(size - inset - arm, size - inset, size - inset, size - inset);
    painter.drawLine(size - inset, size - inset - arm, size - inset, size - inset);

    return QIcon(pixmap);
}

} // namespace

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
    // Song Library sequences only (their own .seq file) - a project-embedded
    // sequence is saved with the project instead.
    QAction *saveAction = nullptr;
    QAction *saveSeparator = nullptr;
    QAction *playAction = nullptr;
    ClipStructureViewer *curvePropertyEditor;
    QTabWidget *detailsTabWidget;
    QWidget *effectEditorContainer;
    QWidget *effectEditor = nullptr;
    // The clip graph's scene while one is open, so clearEditor() can stop
    // recording its selection before tearing it down.
    QPointer<keira::Scene> graphScene;
    // Per clip (by uniqueId): which graph was open (the content graph or a
    // subgraph inside it) and which nodes were selected there, so reopening
    // the clip's graph puts the user back where they were.
    struct GraphView { QByteArray graphId; QByteArrayList nodeIds; };
    QHash<QByteArray, GraphView> graphViews;
    SequenceWaveformEditor *waveform = nullptr;
    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;
    TimelineScene *scene;
    QElapsedTimer timer;
    QVector<SequenceClip*> selectedClips;
    qint64 startTimeMS;
    double lastCurrentTime = 0;
    double currentTime = 0;
    double lastPreviewTime = -1;
    double offset = 0;
    double scale = 20.0;
    bool isPlaying = false;
    bool vdjSyncEnabled = false;
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
    // Scopes the QTabWidget/QTabBar dark-theme rule in styles.css - a plain
    // QTabWidget is otherwise unstyled (native look) everywhere else in the
    // app, since this is the first place one's used.
    setObjectName("sequenceWidget");

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
    connect(m_impl->waveformHeader, &WaveformHeader::previewMarkersChanged,
            m_impl->waveform, &SequenceWaveformEditor::setPreviewMarkers);
    connect(m_impl->waveformHeader, &WaveformHeader::cutMarkersRequested,
            m_impl->waveform, &SequenceWaveformEditor::cutMarkers);
    connect(m_impl->waveformHeader, &WaveformHeader::copyMarkersRequested,
            m_impl->waveform, &SequenceWaveformEditor::copyMarkers);
    connect(m_impl->waveformHeader, &WaveformHeader::pasteMarkersRequested,
            m_impl->waveform, &SequenceWaveformEditor::pasteMarkers);
    //m_impl->viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_impl->viewer->setScene(m_impl->scene);
    m_impl->viewer->setScale(m_impl->scale);

    m_impl->details = new TimelineHeader;
    m_impl->curvePropertyEditor = new ClipStructureViewer;

    m_impl->effectEditorContainer = new QWidget;

    // The waveform normally lives in a real layout inside this container
    // (see showDefaultEditor(), called at the end of this constructor) and
    // is fully interactive there. The one exception is while a channel
    // effect is selected, where it's taken out of the layout, hidden, and
    // painted tinted-down as the curve editor's own background instead (see
    // EffectEditorViewer::drawBackgroundColor/Number) - this event filter
    // keeps it geometry-synced to effectEditorContainer's size for that
    // hidden/unlayouted state, since visibleEndTime()/setOffset()'s pan
    // clamp/zoomToFitSong() all read its width regardless of which state
    // it's currently in.
    m_impl->effectEditorContainer->installEventFilter(this);

    m_impl->detailsTabWidget = new QTabWidget;
    m_impl->detailsTabWidget->addTab(m_impl->curvePropertyEditor, "Channels");
    m_impl->detailsTabWidget->addTab(m_impl->waveformHeader, "Cues");

    m_impl->timeSplitter->addWidget(m_impl->timeToolBar);
    m_impl->timeSplitter->addWidget(m_impl->timebar);



    m_impl->player = new QMediaPlayer(this);
    m_impl->audioOutput = new QAudioOutput(this);
    m_impl->player->setAudioOutput(m_impl->audioOutput);


    // Icon-only actions throughout - tooltips carry the label instead. The
    // playback pair (Rewind, Play) is grouped together first, separated from
    // the view and sync controls that follow.
    m_impl->saveAction = m_impl->timeToolBar->addAction(lightIcon(style(), QStyle::SP_DialogSaveButton), QString());
    m_impl->saveAction->setToolTip("Save Sequence");
    connect(m_impl->saveAction, &QAction::triggered, this, [this](){
        if(Sequence *current = sequence(); current && current->isLibrarySequence())
            current->save(current->filePath());
    });
    m_impl->saveSeparator = m_impl->timeToolBar->addSeparator();
    m_impl->saveAction->setVisible(false);
    m_impl->saveSeparator->setVisible(false);

    auto rewindAction = m_impl->timeToolBar->addAction(lightIcon(style(), QStyle::SP_MediaSkipBackward), QString());
    rewindAction->setToolTip("Rewind");
    connect(rewindAction, &QAction::triggered, this, &SequenceWidget::rewind);

    m_impl->playAction = m_impl->timeToolBar->addAction(lightIcon(style(), QStyle::SP_MediaPlay), QString());
    m_impl->playAction->setToolTip("Play");
    m_impl->playAction->setShortcut(Qt::Key_Space);
    m_impl->playAction->setCheckable(true);
    connect(m_impl->playAction, &QAction::toggled, this, &SequenceWidget::togglePlay);

    m_impl->timeToolBar->addSeparator();

    auto zoomToFitAction = m_impl->timeToolBar->addAction(zoomToFitIcon(), QString());
    zoomToFitAction->setToolTip("Zoom to Fit");
    connect(zoomToFitAction, &QAction::triggered, this, &SequenceWidget::zoomToFitSong);

    m_impl->timeToolBar->addSeparator();

    // VDJ Sync: while checked, Photon's transport (scrub, play/pause, rewind) is
    // mirrored to VirtualDJ - independent of Capture, useful any time during editing.
    auto vdjSyncAction = m_impl->timeToolBar->addAction(lightIcon(style(), QStyle::SP_BrowserReload), QString());
    vdjSyncAction->setToolTip("VDJ Sync");
    vdjSyncAction->setCheckable(true);
    connect(vdjSyncAction, &QAction::toggled, this, &SequenceWidget::toggleVdjSync);



    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->timeSplitter);
    vLayout->addWidget(m_impl->horizontalSplitter);

    //m_impl->scene->setSceneRect(0,0,300,100);

    m_impl->verticalSplitter->addWidget(m_impl->viewer);
    m_impl->verticalSplitter->addWidget(m_impl->effectEditorContainer);

    //m_impl->viewer->centerOn(0,0);

    m_impl->detailsSplitter->addWidget(m_impl->details);
    m_impl->detailsSplitter->addWidget(m_impl->detailsTabWidget);

    m_impl->horizontalSplitter->addWidget(m_impl->detailsSplitter);
    m_impl->horizontalSplitter->addWidget(m_impl->verticalSplitter);
    setLayout(vLayout);
    connect(photonApp->timekeeper(), &Timekeeper::tick, this, &SequenceWidget::tick);
    // Active layer: the headers and the timeline lanes both set and show it.
    connect(m_impl->details, &TimelineHeader::layerActivated, m_impl->scene, [this](photon::Layer *layer){
        if(auto *clipLayer = dynamic_cast<ClipLayer*>(layer))
            m_impl->scene->setActiveLayer(clipLayer);
    });
    connect(m_impl->scene, &TimelineScene::activeLayerChanged, m_impl->details, &TimelineHeader::setActiveLayer);
    connect(m_impl->viewer->verticalScrollBar(), &QAbstractSlider::valueChanged, m_impl->details, &TimelineHeader::offsetChanged);
    connect(m_impl->viewer, &TimelineViewer::scaleChanged, this, &SequenceWidget::setScale);

    connect(m_impl->scene, &TimelineScene::selectionChanged, this, &SequenceWidget::selectionChanged);
    connect(m_impl->verticalSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::editorSplitterMoved);
    connect(m_impl->detailsSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::detailsSplitterMoved);
    connect(m_impl->horizontalSplitter, &QSplitter::splitterMoved, this, &SequenceWidget::horizontalSplitterMoved);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectEffect, this, &SequenceWidget::selectEffect);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectClipGraph, this, &SequenceWidget::selectClipGraph);
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::selectClipProperties, this, [](photon::Clip *clip){
        PropertyController::instance()->selectClip(clip);
    });
    connect(m_impl->curvePropertyEditor, &ClipStructureViewer::clearSelection, this, [this](){
        showDefaultEditor();
        // Its parameter page in the Properties panel goes with it (and a
        // clip's own page, which only stays up while its row is selected).
        auto *subject = PropertyController::instance()->subject();
        if(dynamic_cast<ChannelEffectPropertySubject*>(subject) || dynamic_cast<ClipPropertySubject*>(subject))
            PropertyController::instance()->clear();
    });
    connect(m_impl->timebar, &TimeBar::changeTime, this, &SequenceWidget::gotoTime);
    connect(m_impl->viewer, &TimelineViewer::offsetChanged, this, &SequenceWidget::setOffset);
    connect(m_impl->details, &TimelineHeader::editLayer, this, &SequenceWidget::editLayer);

    connect(m_impl->player, &QMediaPlayer::positionChanged, this, &SequenceWidget::positionChanged);
    connect(m_impl->waveform, &WaveformWidget::visibleRangeChanged, this, &SequenceWidget::waveformRangeChanged);

    // Nothing is selected yet - show the waveform itself rather than an
    // empty panel, same as whenever the selection is cleared later.
    showDefaultEditor();
}

SequenceWidget::~SequenceWidget()
{
    delete m_impl;
}

void SequenceWidget::setSequence(Sequence *t_sequence)
{
    const bool ownFile = t_sequence && t_sequence->isLibrarySequence();
    m_impl->saveAction->setVisible(ownFile);
    m_impl->saveSeparator->setVisible(ownFile);

    m_impl->scene->setSequence(t_sequence);
    //m_impl->viewer->centerOn(0,0);
    m_impl->details->setSequence(t_sequence);
    m_impl->details->setActiveLayer(m_impl->scene->activeLayer());
    m_impl->player->setSource(t_sequence->filePath());
    m_impl->waveform->setSequence(t_sequence);
    m_impl->waveformHeader->setSequence(t_sequence);

    // setSequence() (re)loads the waveform's audio/feature data, which leaves it at
    // its own internal default zoom/offset - align it to whatever range this widget
    // is already showing, the same way every other scale/data change here does.
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

Sequence *SequenceWidget::sequence() const
{
    return m_impl->scene->sequence();
}

void SequenceWidget::editLayer(photon::Layer *t_layer)
{
    clearEditor();

    // A layer's own editor is an arbitrary widget, not one this class can
    // paint the waveform tinted behind the way it does for a channel effect
    // - hide it here rather than leave it sitting unlaid-out underneath.
    m_impl->waveform->hide();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    m_impl->effectEditor = t_layer->createEditor();
    layout->addWidget(m_impl->effectEditor);

    m_impl->effectEditorContainer->setLayout(layout);
}

void SequenceWidget::setScale(double t_scale)
{
    // The coordinator owns the scale clamp so every view agrees on the limit and
    // none can diverge at the zoom boundary.
    t_scale = std::max(t_scale, 2.0);
    m_impl->scale = t_scale;
    m_impl->timebar->setScale(t_scale);
    m_impl->viewer->setScale(t_scale);

    ChannelEffectEditor *channelEditor = dynamic_cast<ChannelEffectEditor*>(m_impl->effectEditor);
    if(channelEditor)
        channelEditor->setXScale(t_scale);
    // Re-clamps the current offset against the new scale and repaints every
    // view from it - the pan-near-song clamp in setOffset() depends on scale
    // (the allowed range is in pixels but scales with px/sec), so a position
    // that was valid before this zoom may not be any more.
    setOffset(m_impl->offset);
}


void SequenceWidget::setScalePoint(QPointF t_scale)
{
    t_scale.setX(std::max(t_scale.x(), 2.0));
    m_impl->scale = t_scale.x();
    m_impl->timebar->setScale(t_scale.x());
    m_impl->viewer->setScale(t_scale.x());

    ChannelEffectEditor *channelEditor = dynamic_cast<ChannelEffectEditor*>(m_impl->effectEditor);
    if(channelEditor)
        channelEditor->setScale(t_scale);
    // See setScale()'s matching comment - the offset's valid range depends on
    // scale, so it needs re-clamping here too.
    setOffset(m_impl->offset);
}

void SequenceWidget::setOffset(double t_offset)
{
    // The coordinator owns this clamp too (see setScale's matching comment):
    // once there's a song to stay near, never let the visible window drift
    // more than half a screen past either end of it - otherwise a fast or
    // high-resolution scroll (Magic Mouse momentum, a trackpad swipe) can
    // wander arbitrarily far from the song in a couple of seconds. With no
    // song data at all there's nothing to stay near, so panning is
    // unbounded, as it always was.
    const double duration = m_impl->waveform->totalDuration();
    const double width = m_impl->waveform->width();
    if(duration > 0.0 && width > 0.0)
    {
        const double minOffset = -0.5 * width;
        const double maxOffset = duration * m_impl->scale - 0.5 * width;
        t_offset = std::clamp(t_offset, minOffset, maxOffset);
    }

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

    // The curve editor paints this itself, tinted, as its own background
    // (see setBackgroundWaveform below) - shown standalone here it would
    // just be redundant, and would steal the mouse events the curve/gizmo
    // handles need.
    m_impl->waveform->hide();

    auto editor = t_effect->createEditor();
    editor->setOffset(m_impl->offset);
    editor->setScale(QPointF(m_impl->scale,editor->scale().y()));
    editor->setBackgroundWaveform(m_impl->waveform);
    connect(editor, &ChannelEffectEditor::offsetChanged, this, &SequenceWidget::setOffset);
    //connect(editor, &ChannelEffectEditor::scaleChanged, m_impl->viewer, &TimelineViewer::setScale);
    connect(editor, &ChannelEffectEditor::scaleChanged, this, &SequenceWidget::setScalePoint);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(editor);

    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);

    // The curve/gizmo editor above stays inline (it needs this widget's live
    // pan/zoom to line handles up with the timeline); the effect's plain
    // parameter fields go to the Properties panel, same as every other
    // selectable thing in the app.
    PropertyController::instance()->selectChannelEffect(t_effect);

    //editor->selectEffect(t_effect);
}

void SequenceWidget::selectClipGraph(photon::Clip *t_clip)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    keira::Graph *graph = t_clip->contentGraph();
    if(graph)
    {
        // The one state that hides the waveform outright - a node graph has
        // nothing to do with the timeline's audio.
        m_impl->waveform->hide();

        auto *library = photonApp->plugins()->nodeLibrary();
        auto *graphWidget = new keira::GraphWidget(library);
    // Node selection now drives the app's Properties panel rather than a
    // sidebar inside the graph widget.
    connect(graphWidget, &keira::GraphWidget::nodeSelected, photonApp, [](keira::Node *node){
        PropertyController::instance()->selectNode(node);
    });

        // Parented to the graphWidget so clearEditor()'s delete of the editor
        // widget tears the scene down with it - GraphWidget itself doesn't own it.
        auto *scene = new keira::Scene(graphWidget);
        scene->setNodeLibrary(library);
        scene->setExternalDropInterpreter(&projectResourceDropInterpreter);
        scene->setGraph(graph);
        // A clip's content graph is already evaluated for real by the clip's own
        // processChannels() (via the sequence's eval thread) whenever it's active -
        // Scene's own GraphEvaluator ticking the SAME graph objects concurrently on
        // ITS thread is redundant and unsafe: individual nodes with their own
        // mutable per-evaluation state (e.g. FixtureStateNode's exposed-input
        // history deques) aren't designed to be evaluate()'d from two threads at
        // once, and it crashes. drainCommandQueue() still runs every tick regardless
        // (see EvalWorker::tick()), so structural edits made here still apply and
        // show up immediately - only the redundant evaluate()/live-value-refresh is
        // disabled.
        scene->setIsAutoEvaluate(false);
        graphWidget->setScene(scene);

        restoreGraphView(t_clip, graphWidget);

        // From here on, track where the user goes: selecting nodes, or
        // navigating into or out of a subgraph.
        m_impl->graphScene = scene;
        const QByteArray clipId = t_clip->uniqueId();
        auto record = [this, scene, clipId](){
            Impl::GraphView view;
            if(scene->graph())
                view.graphId = scene->graph()->uniqueId();
            for(auto *item : scene->selectedItems())
            {
                if(auto *nodeItem = dynamic_cast<keira::NodeItem*>(item))
                    view.nodeIds.append(nodeItem->node()->uniqueId());
            }
            m_impl->graphViews.insert(clipId, view);
        };
        connect(scene, &QGraphicsScene::selectionChanged, this, record);
        connect(scene, &keira::Scene::graphUpdated, this, record);

        layout->addWidget(graphWidget);
        m_impl->effectEditor = graphWidget;
        m_impl->effectEditorContainer->setLayout(layout);
    }
}

namespace {

// The graph with this uniqueId: the given one or any subgraph nested in it.
keira::Graph *findGraph(keira::Graph *t_graph, const QByteArray &t_id)
{
    if(!t_graph)
        return nullptr;
    if(t_graph->uniqueId() == t_id)
        return t_graph;
    for(auto *node : t_graph->nodes())
    {
        if(auto *subGraph = dynamic_cast<keira::SubGraphNode*>(node))
        {
            if(auto *found = findGraph(subGraph->graph(), t_id))
                return found;
        }
    }
    return nullptr;
}

} // namespace

void SequenceWidget::restoreGraphView(Clip *t_clip, keira::GraphWidget *t_graphWidget)
{
    const auto it = m_impl->graphViews.constFind(t_clip->uniqueId());
    keira::Scene *scene = t_graphWidget->scene();
    if(it == m_impl->graphViews.constEnd() || !scene)
        return;

    // Back into the subgraph the user was in, if it's still there.
    if(keira::Graph *graph = findGraph(t_clip->contentGraph(), it->graphId))
        t_graphWidget->navigateToGraph(graph);

    // Reselecting the nodes also puts the last one's page in the Properties
    // panel (GraphWidget::nodeSelected). Nodes deleted since are skipped.
    for(const QByteArray &id : it->nodeIds)
    {
        keira::Node *node = scene->graph() ? scene->graph()->findNode(id) : nullptr;
        if(auto *item = node ? scene->itemForNode(node) : nullptr)
            item->setSelected(true);
    }
}

void SequenceWidget::clearEditor()
{
    // The scene's own teardown deselects everything - that's not the user
    // changing the clip's graph view.
    if(m_impl->graphScene)
        m_impl->graphScene->disconnect(this);
    m_impl->graphScene = nullptr;

    if(m_impl->effectEditorContainer->layout())
        delete m_impl->effectEditorContainer->layout();
    if(m_impl->effectEditor)
        delete m_impl->effectEditor;

    m_impl->effectEditor = nullptr;
}

void SequenceWidget::showDefaultEditor()
{
    clearEditor();

    // With nothing selected to edit, the waveform becomes this panel's own
    // visible content instead of an empty pane - a real, laid-out, visible
    // widget rather than the hidden background layer it is while a channel
    // effect is selected, so it's fully interactive here (cue points can be
    // selected/dragged directly on it).
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_impl->waveform);
    m_impl->effectEditorContainer->setLayout(layout);
    m_impl->waveform->show();
}

void SequenceWidget::selectionChanged()
{
    const auto newSelection = m_impl->scene->selectedItems();
    SequenceClip *previousPrimary = m_impl->selectedClips.isEmpty() ? nullptr : m_impl->selectedClips.last();

    // Kept in selection order, so the property editor can follow the most
    // recently selected clip that's still selected.
    for(auto it = m_impl->selectedClips.begin(); it != m_impl->selectedClips.end();)
    {
        if(newSelection.contains(*it))
            ++it;
        else
            it = m_impl->selectedClips.erase(it);
    }
    for(auto item : newSelection)
    {
        auto *clip = dynamic_cast<SequenceClip*>(item);
        if(clip && !m_impl->selectedClips.contains(clip))
            m_impl->selectedClips.append(clip);
    }

    SequenceClip *primary = m_impl->selectedClips.isEmpty() ? nullptr : m_impl->selectedClips.last();
    if(primary == previousPrimary)
        return;

    // The Properties panel follows whatever view the clip reopens on (its
    // own row shows the clip's properties - see ClipStructureViewer).
    m_impl->curvePropertyEditor->setClip(primary ? primary->clip() : nullptr);
    m_impl->curvePropertyEditor->restoreState();

    if(!primary && dynamic_cast<ClipPropertySubject*>(PropertyController::instance()->subject()))
        PropertyController::instance()->clear();
}

void SequenceWidget::gotoTime(double t_time)
{
    m_impl->currentTime = t_time;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();
    m_impl->lastCurrentTime = t_time;
    m_impl->timer.restart();
    sequence()->setPreviewTime(m_impl->currentTime);
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);

    m_impl->player->setPosition(m_impl->currentTime*1000);
    m_impl->waveform->setPlayhead(m_impl->currentTime);

    if(m_impl->vdjSyncEnabled)
        photonApp->djConnector()->sendSeek(t_time);
}

void SequenceWidget::positionChanged(qint64 t_time)
{
    if(!m_impl->isPlaying)
        return;

    // tick() interpolates currentTime from wall-clock elapsed time rather than
    // querying the player every frame (smoother than QMediaPlayer's own position
    // update rate) - but the system clock and the audio hardware's playback
    // clock don't run at exactly the same rate, so that interpolation slowly
    // drifts from what's actually audible. Rebase it to the player's real
    // position on every update it reports, so drift never accumulates beyond
    // one update interval.
    m_impl->lastCurrentTime = t_time / 1000.0;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();
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
    sequence()->setPreviewTime(m_impl->currentTime);
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
    m_impl->waveform->setPlayhead(m_impl->currentTime);
}

void SequenceWidget::waveformRangeChanged(double start, double end)
{
    // The waveform was panned/zoomed directly — convert its new visible time range
    // into the shared scale/offset so every view follows. setScale clamps, and the
    // resulting frameTime() call re-aligns the waveform to the clamped values.
    if(end <= start)
        return;
    const double w = m_impl->waveform->width();
    if(w <= 0)
        return;

    setScale(w / (end - start));
    setOffset(start * m_impl->scale);
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
    sequence()->setPreviewTime(m_impl->currentTime);

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

    // Keep the toolbar button in sync (icon + checked state) regardless of
    // whether this was reached via the action itself or another path (e.g.
    // SequencePanel's own Space-bar handler calling this directly).
    if(m_impl->playAction)
    {
        m_impl->playAction->setIcon(lightIcon(style(), t_value ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
        m_impl->playAction->setToolTip(t_value ? "Pause" : "Play");
        if(m_impl->playAction->isChecked() != t_value)
        {
            const QSignalBlocker blocker(m_impl->playAction);
            m_impl->playAction->setChecked(t_value);
        }
    }

    m_impl->lastCurrentTime = m_impl->currentTime;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();

    if(m_impl->isPlaying)
        m_impl->player->play();
    else
        m_impl->player->pause();

    if(m_impl->vdjSyncEnabled)
    {
        if(m_impl->isPlaying)
        {
            // Make sure VDJ starts from exactly where Photon is showing, not wherever
            // it happened to be left - sync may not have been the thing that put it
            // there (e.g. it was only just enabled).
            photonApp->djConnector()->sendSeek(m_impl->currentTime);
            photonApp->djConnector()->sendPlay();
        }
        else
        {
            photonApp->djConnector()->sendPause();
        }
    }
}

void SequenceWidget::rewind()
{
    m_impl->currentTime = 0.0;
    m_impl->lastCurrentTime = 0.0;
    m_impl->startTimeMS = QDateTime::currentMSecsSinceEpoch();
    sequence()->setPreviewTime(m_impl->currentTime);
    m_impl->viewer->movePlayheadTo(m_impl->currentTime);
    m_impl->player->setPosition(0);

    if(m_impl->vdjSyncEnabled)
        photonApp->djConnector()->sendRestart();
}

void SequenceWidget::zoomToFitSong()
{
    const double duration = m_impl->waveform->totalDuration();
    if(duration > 0.0)
    {
        const double width = m_impl->waveform->width();
        if(width <= 0.0)
            return;
        // setScale clamps to a minimum zoom (currently 2.0 px/sec), so a song
        // long enough to need less than that to fit ends up framed from the
        // start rather than fully - the same floor every other zoom path
        // already respects, not something this button should bypass.
        setScale(width / duration);
        setOffset(0.0);
    }
    else
    {
        // Nothing to fit - just return to the start, same as Rewind but for
        // the view instead of the playhead.
        setOffset(0.0);
    }
}

void SequenceWidget::toggleVdjSync(bool t_value)
{
    m_impl->vdjSyncEnabled = t_value;
}

void SequenceWidget::showEvent(QShowEvent*t_event)
{
    QWidget::showEvent(t_event);

    int halfHeight = height() / 2;
    m_impl->horizontalSplitter->setSizes({static_cast<int>(width()*.2),static_cast<int>(width()*.8)});
    m_impl->verticalSplitter->setSizes({halfHeight,halfHeight});
    m_impl->detailsSplitter->setSizes({halfHeight,halfHeight});
    m_impl->timeSplitter->setSizes(m_impl->horizontalSplitter->sizes());
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

void SequenceWidget::resizeEvent(QResizeEvent* t_event)
{
    QWidget::resizeEvent(t_event);
    m_impl->waveform->frameTime(m_impl->visibleStartTime(), m_impl->visibleEndTime());
}

bool SequenceWidget::eventFilter(QObject *t_watched, QEvent *t_event)
{
    // Keeps the (now hidden, unlayouted) waveform sized to match whatever's
    // showing it - dragging a splitter handle resizes effectEditorContainer
    // without this widget's own resizeEvent firing, so that alone isn't
    // enough to catch it.
    if(t_watched == m_impl->effectEditorContainer && t_event->type() == QEvent::Resize)
        m_impl->waveform->resize(m_impl->effectEditorContainer->size());

    return QWidget::eventFilter(t_watched, t_event);
}

} // namespace photon

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
#include <QPushButton>
#include <QMenu>
#include "ActionEditWidget.h"
#include "ActionEditWidget.h"
#include "photoncore.h"
#include "actionstructureviewer.h"
#include "sequence/channeleffect.h"
#include "sequence/clip.h"
#include "state/state.h"
#include "falloff/falloffeffect.h"
#include "graph/bus/busevaluator.h"
#include "sequence/viewer/stateeditor.h"
#include "sequence/fixtureclip.h"
#include "sequence/baseeffect.h"
#include "fixture/maskeffect.h"
#include "surface/fixtureaction.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/surfaceaction.h"
#include "surface/canvasaction.h"
#include "project/project.h"
#include "pixel/canvas.h"
#include "pixel/canvascollection.h"

namespace photon {

class ActionEditWidget::Impl
{
public:

    QSplitter *horizontalSplitter;
    ActionStructureViewer *structureViewer;
    QWidget *effectEditorContainer;
    QWidget *effectEditor = nullptr;
    QPushButton *addButton;
    QPushButton *removeButton;
    SurfaceGizmoContainer *container = nullptr;
    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;
    SurfaceAction *selectedAction = nullptr;
    QElapsedTimer timer;
    QVector<SurfaceAction*> selectedActions;
    qint64 startTimeMS;
    double lastCurrentTime = 0;
    double currentTime = 0;
    double lastPreviewTime = -1;
    double offset = 0;
    double scale = 20.0;
    bool isPlaying = false;
};


ActionEditWidget::ActionEditWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->horizontalSplitter = new QSplitter;

    //m_impl->viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    m_impl->structureViewer = new ActionStructureViewer;

    m_impl->effectEditorContainer = new QWidget;


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->horizontalSplitter);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0,0,0,0);
    m_impl->addButton = new QPushButton("Add");
    m_impl->removeButton = new QPushButton("Remove");
    m_impl->removeButton->setEnabled(false);
    buttonLayout->addWidget(m_impl->addButton);
    buttonLayout->addWidget(m_impl->removeButton);

    connect(m_impl->addButton, &QPushButton::clicked, this, &ActionEditWidget::addAction);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &ActionEditWidget::removeAction);

    QVBoxLayout *leftSideLayout = new QVBoxLayout;
    leftSideLayout->setContentsMargins(0,0,0,0);

    leftSideLayout->addWidget(m_impl->structureViewer);
    leftSideLayout->addLayout(buttonLayout);

    QWidget *w = new QWidget;
    w->setLayout(leftSideLayout);

    m_impl->horizontalSplitter->addWidget(w);
    m_impl->horizontalSplitter->addWidget(m_impl->effectEditorContainer);
    setLayout(vLayout);


    connect(m_impl->structureViewer, &ActionStructureViewer::selectFalloff, this, &ActionEditWidget::selectFalloff);
    connect(m_impl->structureViewer, &ActionStructureViewer::selectState, this, &ActionEditWidget::selectState);
    connect(m_impl->structureViewer, &ActionStructureViewer::selectEffect, this, &ActionEditWidget::selectEffect);
    connect(m_impl->structureViewer, &ActionStructureViewer::selectActionParameter, this, &ActionEditWidget::selectActionParameter);
    connect(m_impl->structureViewer, &ActionStructureViewer::selectMask, this, &ActionEditWidget::selectMask);
    connect(m_impl->structureViewer, &ActionStructureViewer::selectBaseEffect, this, &ActionEditWidget::selectBaseEffect);
    connect(m_impl->structureViewer, &ActionStructureViewer::selectAction, this, &ActionEditWidget::selectAction);
    connect(m_impl->structureViewer, &ActionStructureViewer::clearSelection, this, &ActionEditWidget::clearEditor);

    setMinimumSize(1200,800);


}

ActionEditWidget::~ActionEditWidget()
{
    delete m_impl;
}

void ActionEditWidget::setContainer(SurfaceGizmoContainer *t_container)
{
    m_impl->container = t_container;
    m_impl->structureViewer->setContainer(t_container);
}

SurfaceGizmoContainer *ActionEditWidget::container() const
{
    return m_impl->container;
}


void ActionEditWidget::selectEffect(photon::ChannelEffect *t_effect)
{

    clearEditor();

    auto editor = t_effect->createEditor();
    editor->setOffset(m_impl->offset);
    editor->setScale(QPointF(m_impl->scale,editor->scale().y()));
    //connect(editor, &ChannelEffectEditor::scaleChanged, m_impl->viewer, &TimelineViewer::setScale);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(editor);

    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);


    //editor->selectEffect(t_effect);
}

void ActionEditWidget::selectMask(photon::MaskEffect *t_effect)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = t_effect->createEditor();
    layout->addWidget(editor);
    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);
}

void ActionEditWidget::selectFalloff(photon::FalloffEffect *t_effect)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = t_effect->createEditor();
    layout->addWidget(editor);
    m_impl->effectEditor = editor;
    m_impl->effectEditorContainer->setLayout(layout);

}

void ActionEditWidget::selectBaseEffect(photon::BaseEffect *t_effect)
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

void ActionEditWidget::selectActionParameter(photon::SurfaceAction *t_clip)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = t_clip->widget();

    if(editor)
    {
        layout->addWidget(editor);
        m_impl->effectEditor = editor;
        m_impl->effectEditorContainer->setLayout(layout);
    }
}

void ActionEditWidget::selectState(photon::State *t_state)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    auto editor = new StateEditor;

    //if(m_impl->selectedActions.length() > 0)
    {
        auto clip = dynamic_cast<FixtureAction *>(t_state->parent());
        if(clip)
            editor->setBaseEffectParent(clip);
    }

    layout->addWidget(editor);
    m_impl->effectEditor = editor;

    m_impl->effectEditorContainer->setLayout(layout);

}

void ActionEditWidget::clearEditor()
{
    if(m_impl->effectEditorContainer->layout())
        delete m_impl->effectEditorContainer->layout();
    if(m_impl->effectEditor)
        delete m_impl->effectEditor;

    m_impl->removeButton->setEnabled(false);
    m_impl->effectEditor = nullptr;
    m_impl->selectedAction = nullptr;
}

void ActionEditWidget::selectAction(photon::SurfaceAction *t_action)
{
    clearEditor();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    m_impl->removeButton->setEnabled(true);

    QWidget *settingsWidget = t_action->settingsWidget();

    layout->addWidget(settingsWidget);
    m_impl->effectEditor = settingsWidget;

    m_impl->effectEditorContainer->setLayout(layout);

    m_impl->selectedAction = t_action;
}

void ActionEditWidget::selectionChanged()
{

}

void ActionEditWidget::addAction()
{
    QMenu menu;
    menu.addAction("Fixture",[this](){m_impl->container->addAction(new FixtureAction);});
    menu.addAction("Canvas",[this](){

        auto action = new CanvasAction;
        if(photonApp->project()->canvases()->canvasCount() != 0)
            action->setCanvas(photonApp->project()->canvases()->canvases()[0]);
        m_impl->container->addAction(action);
    });

    menu.exec(m_impl->addButton->mapToGlobal(m_impl->addButton->rect().bottomLeft()));



}

void ActionEditWidget::removeAction()
{
    if(m_impl->selectedAction)
        m_impl->container->removeAction(m_impl->selectedAction);
    clearEditor();
}

void ActionEditWidget::showEvent(QShowEvent*t_event)
{
    QWidget::showEvent(t_event);
    m_impl->horizontalSplitter->setSizes({static_cast<int>(width()*.2),static_cast<int>(width()*.8)});

}

void ActionEditWidget::resizeEvent(QResizeEvent* t_event)
{
    QWidget::resizeEvent(t_event);
}

} // namespace photon

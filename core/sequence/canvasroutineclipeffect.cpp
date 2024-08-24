#include <QVBoxLayout>
#include "canvasroutineclipeffect.h"
#include "routine/routine.h"
#include "clip.h"
#include "routine/routineevaluationcontext.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"
#include "routine/node/numberinputnode.h"
#include "routine/node/colorinputnode.h"
#include "model/parameter/parameter.h"
#include "channel/parameter/point2channelparameter.h"

namespace photon {


RoutineEditor::RoutineEditor(Routine *t_routine)
{
    viewer = new keira::Viewer;

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(viewer);
    setLayout(vLayout);


    scene = new keira::Scene;
    scene->setIsAutoEvaluate(false);
    scene->setGraph(t_routine);
    scene->setNodeLibrary(photonApp->plugins()->nodeLibrary());

    viewer->setScene(scene);
}

RoutineEditor::~RoutineEditor()
{
    delete scene;
}




class CanvasRoutineClipEffect::Impl
{
public:
    Routine *routine;
    QVector<keira::Parameter*> inputs;
};

CanvasRoutineClipEffect::CanvasRoutineClipEffect():CanvasClipEffect(),m_impl(new Impl)
{
    m_impl->routine = new Routine;

    connect(m_impl->routine, &Routine::channelAdded, this, &CanvasRoutineClipEffect::channelAddedSlot);
    connect(m_impl->routine, &Routine::channelRemoved, this, &CanvasRoutineClipEffect::channelRemovedSlot);
    connect(m_impl->routine, &Routine::channelUpdated, this, &CanvasRoutineClipEffect::routineChannelUpdatedSlot);
    connect(m_impl->routine, &Routine::nodeWasAdded, this, &CanvasRoutineClipEffect::nodeWasAdded);
    connect(m_impl->routine, &Routine::nodeWasRemoved, this, &CanvasRoutineClipEffect::nodeWasRemoved);
}

CanvasRoutineClipEffect::~CanvasRoutineClipEffect() {
    delete m_impl->routine;
    delete m_impl;
}

void CanvasRoutineClipEffect::routineChannelUpdatedSlot(int t_index)
{
    channels()[t_index]->updateInfo(m_impl->routine->channelInfoAtIndex(t_index));
}

void CanvasRoutineClipEffect::channelAddedSlot(int t_index)
{
    qDebug() << "add channel" << t_index << channelCount() << m_impl->routine->channelCount();
    const auto &info = m_impl->routine->channelInfoAtIndex(t_index);

    if(info.type == ChannelInfo::ChannelTypePoint)
    {
        auto *param = new Point2ChannelParameter(info.name);
        createChannelsFromParameter(param, ChannelInfo::ChannelTypeNumber);
    }
    else
        addChannel(m_impl->routine->channelInfoAtIndex(t_index), t_index);
}

void CanvasRoutineClipEffect::channelRemovedSlot(int t_index)
{
    const auto &info = m_impl->routine->channelInfoAtIndex(t_index);
    if(info.type == ChannelInfo::ChannelTypePoint)
    {
        for(auto param : parameters()->channelParameters())
        {
            if(info.name == param->name())
            {
                removeChannelParameter(param);
                return;
            }
        }
    }
    else
        removeChannel(t_index);
}

void CanvasRoutineClipEffect::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    CanvasClipEffect::initializeContext(t_context, t_canvas);

    if(m_impl->routine)
        m_impl->routine->initializeContext(t_context, t_canvas);
}

void CanvasRoutineClipEffect::canvasResized(QOpenGLContext *t_context, Canvas *t_canvas)
{
    CanvasClipEffect::canvasResized(t_context, t_canvas);

    if(m_impl->routine)
        m_impl->routine->canvasResized(t_context, t_canvas);
}

Routine *CanvasRoutineClipEffect::routine() const
{
    return m_impl->routine;
}

void CanvasRoutineClipEffect::nodeWasAdded(keira::Node *t_node)
{
    auto inputNode = dynamic_cast<NumberInputNode*>(t_node);
    if(inputNode)
        m_impl->inputs << inputNode->findParameter(NumberInputNode::Value);
    auto colorNode = dynamic_cast<ColorInputNode*>(t_node);
    if(colorNode)
        m_impl->inputs << colorNode->findParameter(ColorInputNode::Value);
}

void CanvasRoutineClipEffect::nodeWasRemoved(keira::Node *t_node)
{
    auto inputNode = dynamic_cast<NumberInputNode*>(t_node);
    if(inputNode)
        m_impl->inputs.removeOne(inputNode->findParameter(NumberInputNode::Value));
    auto colorNode = dynamic_cast<ColorInputNode*>(t_node);
    if(colorNode)
        m_impl->inputs.removeOne(colorNode->findParameter(ColorInputNode::Value));
}

QWidget *CanvasRoutineClipEffect::createEditor()
{
    return new RoutineEditor(m_impl->routine);
}

void CanvasRoutineClipEffect::evaluate(CanvasClipEffectEvaluationContext &t_context)
{
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    RoutineEvaluationContext localContext(t_context.dmxMatrix);
    localContext.globalTime = t_context.globalTime;
    localContext.relativeTime = initialRelativeTime;
    localContext.project = t_context.project;
    localContext.openglContext = t_context.openglContext;
    localContext.frameBuffer = t_context.buffer;
    localContext.resources = t_context.resources;
    localContext.canvas = t_context.canvas;


    t_context.relativeTime = initialRelativeTime;
    if(t_context.relativeTime < 0)
        return;

    t_context.strength = clip()->strengthAtTime(t_context.relativeTime);

    localContext.channelValues.clear();
    for(const auto &channel : channels())
    {
        if(ChannelInfo::ChannelType::ChannelTypeNumber == channel->type())
            localContext.channelValues.insert(channel->uniqueId(), channel->processValue(t_context.relativeTime));

        else if(ChannelInfo::ChannelType::ChannelTypeColor == channel->type())
            localContext.channelValues.insert(channel->uniqueId(), channel->processValue(t_context.relativeTime));
    }

    for(auto param : m_impl->inputs)
    {
        param->setValue(localContext.channelValues.value(param->node()->uniqueId()));
    }

    m_impl->routine->evaluate(&localContext);
    m_impl->routine->markClean();
}

void CanvasRoutineClipEffect::restore(Project &t_project)
{
    CanvasClipEffect::restore(t_project);


    for(auto node : m_impl->routine->nodes())
    {
        auto inputNode = dynamic_cast<NumberInputNode*>(node);
        if(inputNode)
            m_impl->inputs << inputNode->findParameter(NumberInputNode::Value);
        auto colorNode = dynamic_cast<ColorInputNode*>(node);
        if(colorNode)
            m_impl->inputs << colorNode->findParameter(ColorInputNode::Value);
    }

}

void CanvasRoutineClipEffect::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    CanvasClipEffect::readFromJson(t_json, t_context);

    auto routineObj = t_json.value("routine").toObject();
    m_impl->routine->readFromJson(routineObj, photonApp->plugins()->nodeLibrary());
}

void CanvasRoutineClipEffect::writeToJson(QJsonObject &t_json) const
{
    CanvasClipEffect::writeToJson(t_json);

    QJsonObject routineObj;
    m_impl->routine->writeToJson(routineObj);
    t_json.insert("routine", routineObj);
}


ClipEffectInformation CanvasRoutineClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new CanvasRoutineClipEffect;});
    toReturn.name = "Routine";
    toReturn.id = "photon.clip.effect.Routine";
    toReturn.categories.append("Draw");

    return toReturn;
}


} // namespace photon

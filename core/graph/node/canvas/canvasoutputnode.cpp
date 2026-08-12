#include <QCoreApplication>
#include "canvasoutputnode.h"
#include "canvasdmxsampler.h"
#include "canvassubgraphnode.h"
#include "model/graph.h"
#include "graph/parameter/pixellayoutlistparameter.h"
#include "model/parameter/buttonparameter.h"
#include "rhi/rhicontext.h"
#include "routine/routineevaluationcontext.h"
#include "processcontext.h"
#include "photoncore.h"
#include "gui/guimanager.h"
#include "gui/panel/canvaspreviewpanel.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "pixel/pixelsourcelayout.h"
#include "pixel/pixelsource.h"

namespace photon {

const QByteArray CanvasOutputNode::TextureInput = "canvasInput";
const QByteArray CanvasOutputNode::Layouts = "layouts";
const QByteArray CanvasOutputNode::ViewPreview = "viewPreview";

keira::NodeInformation CanvasOutputNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasOutputNode;});
    toReturn.name = "Canvas Output";
    toReturn.nodeId = "photon.canvas.output";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasOutputNode::CanvasOutputNode() : keira::Node("photon.canvas.output")
{
    setName("Output");
}

CanvasOutputNode::~CanvasOutputNode()
{
    delete m_sampler;   // its dtor defers GPU teardown to the main thread if needed
}

void CanvasOutputNode::createParameters()
{
    m_textureInput = new RhiTextureParameter(TextureInput, "Canvas", RhiTextureData{},
                                             keira::AllowSingleInput);
    addParameter(m_textureInput);

    m_layoutsParam = new PixelLayoutListParameter(Layouts, "Pixel Layouts", {});
    addParameter(m_layoutsParam);

    m_viewButton = new keira::ButtonParameter(ViewPreview, "View Preview");
    addParameter(m_viewButton);
}

void CanvasOutputNode::buttonClicked(const keira::Parameter *t_param)
{
    if (t_param != m_viewButton)
        return;

    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    if (!app || !app->gui())
        return;

    auto *gui = app->gui();
    Panel *panel = gui->findPanel("photon.canvas-preview");
    if (!panel)
        panel = gui->createDockedPanel("photon.canvas-preview");
    if (!panel)
        return;

    gui->bringPanelToFront(panel);
    if (auto *preview = dynamic_cast<CanvasPreviewPanel *>(panel))
        preview->previewOutput(this);
}

QVector<PixelLayout *> CanvasOutputNode::selectedLayouts() const
{
    QVector<PixelLayout *> result;
    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    if (!app || !app->project() || !m_layoutsParam)
        return result;

    for (const auto &id : m_layoutsParam->value().toStringList())
        if (auto *layout = app->project()->pixelLayouts()->findLayoutWithId(id.toLatin1()))
            result << layout;
    return result;
}

QVector<QPointF> CanvasOutputNode::buildSampleUVs() const
{
    QVector<QPointF> uvs;
    for (auto *layout : selectedLayouts())
        for (auto *sourceLayout : layout->sourceLayouts())
            if (sourceLayout->source())
                sourceLayout->collectSampleUVs(uvs);
    return uvs;
}

void CanvasOutputNode::evaluate(keira::EvaluationContext *t_context) const
{
    // The input param holds the upstream producer's handle at eval time.
    m_lastTexture = m_textureInput->value().value<RhiTextureData>();
    m_gatheredThisFrame = false;

    auto *context = dynamic_cast<RoutineEvaluationContext *>(t_context);
    if (!context || !context->rhiContext || !context->rhiCommandBuffer)
        return;
    if (!m_lastTexture.texture)
        return;

    const QVector<QPointF> uvs = buildSampleUVs();
    if (uvs.isEmpty())
        return;

    if (!m_sampler)
        m_sampler = new CanvasDmxSampler;

    // Flatten over the owning canvas's background colour (transparent -> background).
    QColor background(0, 0, 0, 255);
    if (auto *canvas = dynamic_cast<CanvasSubGraphNode *>(graph()->parentNode()))
        background = canvas->backgroundColor();

    // Sample this output's own input texture (a specific point in the flow).
    m_sampler->recordGather(context->rhiContext->rhi(), context->rhiCommandBuffer,
                            m_lastTexture.texture, uvs, background, nullptr);
    m_gatheredThisFrame = true;
}

void CanvasOutputNode::collectGatheredColors() const
{
    if (!m_gatheredThisFrame || !m_sampler)
        return;
    QVector<QColor> colors;
    if (m_sampler->takeColors(colors)) {
        QMutexLocker lock(&m_gatheredMutex);
        m_gatheredColors.swap(colors);
    }
}

void CanvasOutputNode::writeDmx(ProcessContext &context) const
{
    QVector<QColor> colors;
    {
        QMutexLocker lock(&m_gatheredMutex);
        colors = m_gatheredColors;
    }
    if (colors.isEmpty())
        return;

    context.gatheredColors = &colors;
    context.gatheredIndex = 0;
    // Same layout order as buildSampleUVs() so each source pops its own colours.
    for (auto *layout : selectedLayouts())
        layout->process(context);
    context.gatheredColors = nullptr;
}

} // namespace photon

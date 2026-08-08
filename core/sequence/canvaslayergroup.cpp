#include <algorithm>
#include <rhi/qrhi.h>
#include <QThread>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QListWidget>
#include <QMenu>
#include <QMutex>
#include <QMutexLocker>
#include <QSpinBox>
#include <QColorDialog>
#include "canvaslayergroup.h"
#include "sequence.h"
#include "project/project.h"
#include "pixel/pixelsource.h"
#include "pixel/pixellayout.h"
#include "scene/sceneiterator.h"
#include "scene/sceneobject.h"
#include "pixel/pixellayoutcollection.h"
#include "photoncore.h"
#include "canvasclip.h"
#include "graph/node/canvas/canvasrendermanager.h"
#include "graph/node/canvas/canvascompositenode.h"
#include "graph/node/canvas/canvasoutputnode.h"
#include "model/graph.h"
#include "rhi/rhicontext.h"
#include "routine/routineevaluationcontext.h"
#include "gui/guimanager.h"
#include "gui/panel/canvaspreviewpanel.h"

namespace photon {

class CanvasLayerGroupEditor::Impl
{
public:
    CanvasLayerGroup *canvasGroup;
    QListWidget *pixelLayoutList;
    QPushButton *addLayoutButton;
    QPushButton *removeLayoutButton;
    QSpinBox *widthSpin;
    QSpinBox *heightSpin;
    QPushButton *backgroundButton;
    QPushButton *viewPreviewButton;
};

CanvasLayerGroupEditor::CanvasLayerGroupEditor(CanvasLayerGroup *t_group):QWidget(),m_impl(new Impl)
{
    m_impl->canvasGroup = t_group;

    connect(t_group, &CanvasLayerGroup::pixelLayoutAdded, this, &CanvasLayerGroupEditor::pixelLayoutAdded);
    connect(t_group, &CanvasLayerGroup::pixelLayoutRemoved, this, &CanvasLayerGroupEditor::pixelLayoutRemoved);

    QFormLayout *formLayout = new QFormLayout;

    m_impl->widthSpin = new QSpinBox;
    m_impl->widthSpin->setRange(1, 8192);
    m_impl->widthSpin->setValue(t_group->canvasWidth());
    connect(m_impl->widthSpin, &QSpinBox::valueChanged, this, &CanvasLayerGroupEditor::widthChanged);
    formLayout->addRow("Width", m_impl->widthSpin);

    m_impl->heightSpin = new QSpinBox;
    m_impl->heightSpin->setRange(1, 8192);
    m_impl->heightSpin->setValue(t_group->canvasHeight());
    connect(m_impl->heightSpin, &QSpinBox::valueChanged, this, &CanvasLayerGroupEditor::heightChanged);
    formLayout->addRow("Height", m_impl->heightSpin);

    m_impl->backgroundButton = new QPushButton("Background...");
    connect(m_impl->backgroundButton, &QPushButton::clicked, this, [this](){
        QColor color = QColorDialog::getColor(m_impl->canvasGroup->background(), this,
                                              "Background Colour", QColorDialog::ShowAlphaChannel);
        if(color.isValid())
            backgroundChanged(color);
    });
    formLayout->addRow("Background", m_impl->backgroundButton);

    m_impl->viewPreviewButton = new QPushButton("View Preview");
    connect(m_impl->viewPreviewButton, &QPushButton::clicked, this, [this](){
        auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
        if(!app || !app->gui())
            return;

        auto *gui = app->gui();
        Panel *panel = gui->findPanel("photon.canvas-preview");
        if(!panel)
            panel = gui->createDockedPanel("photon.canvas-preview");
        if(!panel)
            return;

        gui->bringPanelToFront(panel);
        if(auto *preview = dynamic_cast<CanvasPreviewPanel *>(panel))
            preview->previewLayerGroup(m_impl->canvasGroup);
    });
    formLayout->addRow("", m_impl->viewPreviewButton);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);

    m_impl->pixelLayoutList = new QListWidget;

    for(auto layout : t_group->pixelLayouts())
    {
        m_impl->pixelLayoutList->addItem(layout->name());
    }


    vLayout->addWidget(m_impl->pixelLayoutList);

    m_impl->addLayoutButton = new QPushButton("Add");
    connect(m_impl->addLayoutButton, &QPushButton::clicked,this, &CanvasLayerGroupEditor::openAddPixelLayout);

    m_impl->removeLayoutButton = new QPushButton("Remove");
    connect(m_impl->removeLayoutButton, &QPushButton::clicked,this, &CanvasLayerGroupEditor::removeSelectedLayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(m_impl->addLayoutButton);
    hLayout->addWidget(m_impl->removeLayoutButton);

    vLayout->addLayout(hLayout);
    formLayout->addRow("Pixel Layouts", vLayout);

    setLayout(formLayout);
}

void CanvasLayerGroupEditor::widthChanged(int t_value)
{
    m_impl->canvasGroup->setCanvasWidth(t_value);
}

void CanvasLayerGroupEditor::heightChanged(int t_value)
{
    m_impl->canvasGroup->setCanvasHeight(t_value);
}

void CanvasLayerGroupEditor::backgroundChanged(QColor t_value)
{
    m_impl->canvasGroup->setBackground(t_value);
}

void CanvasLayerGroupEditor::openAddPixelLayout()
{
    QMenu menu;
    auto pixelLayouts = photonApp->project()->pixelLayouts()->layouts();
    for(auto layout : pixelLayouts)
    {
        if(!m_impl->canvasGroup->pixelLayouts().contains(layout))
            menu.addAction(layout->name(),[this, layout](){m_impl->canvasGroup->addPixelLayout(layout);});
    }
    menu.exec(m_impl->addLayoutButton->mapToGlobal(QPoint(0,m_impl->addLayoutButton->height())));
}

void CanvasLayerGroupEditor::removeSelectedLayout()
{
    QByteArrayList ids;
    for(auto item : m_impl->pixelLayoutList->selectedItems())
    {
        ids << item->data(Qt::ItemDataRole::UserRole).toByteArray();
    }

    for(const auto &id : ids)
    {
        for(auto layout : m_impl->canvasGroup->pixelLayouts())
        {
            if(layout->uniqueId() == id)
                m_impl->canvasGroup->removePixelLayout(layout);
        }
    }

}

void CanvasLayerGroupEditor::pixelLayoutAdded(photon::PixelLayout *t_layout)
{
    auto item = new QListWidgetItem(t_layout->name());
    item->setData(Qt::ItemDataRole::UserRole, t_layout->uniqueId());
    m_impl->pixelLayoutList->addItem(item);
}

void CanvasLayerGroupEditor::pixelLayoutRemoved(photon::PixelLayout *t_layout)
{
    for(int i = 0; i < m_impl->pixelLayoutList->count(); ++i)
    {
        auto item = m_impl->pixelLayoutList->item(i);
        if(item->data(Qt::ItemDataRole::UserRole).toByteArray() == t_layout->uniqueId())
        {
            delete item;
        }
    }
}




class CanvasLayerGroup::Impl
{
public:
    QVector<PixelLayout*> pixelLayouts;
    int width = 256;
    int height = 256;
    QColor background = QColor(0, 0, 0, 255);

    // Non-null in the app (registered for main-thread rendering); null in
    // headless tests, where there's nothing to render into anyway.
    CanvasRenderManager *manager = nullptr;

    // Set by processChannels() (worker or GUI thread), consumed by the
    // manager (main thread).
    std::atomic<bool> needsRender{false};

    // Active-clip snapshot for this frame. queueClipForRender() (called from
    // whichever thread processChannels() runs on) appends to pendingActiveClips;
    // processChannels() clears it before recursing into sub-layers/clips and
    // publishes it to activeClips once they've all reported in. renderMainThread()
    // (main thread) reads activeClips. Guarded because a QVector copy isn't a
    // benign cross-thread read the way the plain doubles elsewhere are.
    QMutex activeClipsMutex;
    QVector<CanvasClipRenderState> pendingActiveClips;
    QVector<CanvasClipRenderState> activeClips;

    // Ping-pong accumulator compositors used to composite active clips onto
    // the sink one at a time without a composite ever reading and writing the
    // same physical texture (see renderMainThread()). Lazily created; owned
    // outside of any graph, driven directly via setValue()/evaluate(). Main-
    // thread only, like the sink resources below.
    CanvasCompositeNode *compositors[2] = {nullptr, nullptr};

    // Owned sink resources. Main-thread only. (Not `mutable` - accessed
    // through the m_impl pointer, which stays non-const regardless of the
    // owning CanvasLayerGroup method's own constness.)
    QRhiTexture *canvasTexture = nullptr;
    QRhiTextureRenderTarget *canvasRT = nullptr;
    QRhiRenderPassDescriptor *canvasRP = nullptr;
    QSize canvasSize;
};

CanvasLayerGroup::CanvasLayerGroup(QObject *t_parent):LayerGroup("CanvasGroup", t_parent),m_impl(new Impl)
{
    m_impl->manager = CanvasRenderManager::instance();
    if(m_impl->manager)
        m_impl->manager->registerCanvas(this);
}

CanvasLayerGroup::CanvasLayerGroup(const QString &t_name, QObject *t_parent):LayerGroup(t_name,"CanvasGroup", t_parent),m_impl(new Impl)
{
    m_impl->manager = CanvasRenderManager::instance();
    if(m_impl->manager)
        m_impl->manager->registerCanvas(this);
}

CanvasLayerGroup::~CanvasLayerGroup()
{
    // First thing, before anything else: once this is set, clipBeingDestroyed()
    // (potentially called back into by child CanvasClips as QObject's parent/
    // child cascade destroys them further down this same destructor chain)
    // knows m_impl may already be gone and won't touch it.
    m_tornDown.store(true);

    // Unregister: blocks until any in-progress main-thread render of this
    // group finishes, so nothing can render it mid-teardown (mirrors
    // CanvasSubGraphNode::~CanvasSubGraphNode()).
    if(m_impl->manager)
        m_impl->manager->unregisterCanvas(this);
    releaseSink();
    delete m_impl->compositors[0];
    delete m_impl->compositors[1];
    delete m_impl;
}

void CanvasLayerGroup::clipBeingDestroyed(CanvasClip *t_clip)
{
    if(m_tornDown.load())
        return;

    QMutexLocker lock(&m_impl->activeClipsMutex);
    auto matches = [t_clip](const CanvasClipRenderState &s){ return s.clip == t_clip; };
    m_impl->pendingActiveClips.erase(std::remove_if(m_impl->pendingActiveClips.begin(), m_impl->pendingActiveClips.end(), matches), m_impl->pendingActiveClips.end());
    m_impl->activeClips.erase(std::remove_if(m_impl->activeClips.begin(), m_impl->activeClips.end(), matches), m_impl->activeClips.end());
}

int CanvasLayerGroup::canvasWidth() const
{
    return m_impl->width;
}

int CanvasLayerGroup::canvasHeight() const
{
    return m_impl->height;
}

void CanvasLayerGroup::setCanvasWidth(int t_value)
{
    m_impl->width = std::max(1, t_value);
}

void CanvasLayerGroup::setCanvasHeight(int t_value)
{
    m_impl->height = std::max(1, t_value);
}

QColor CanvasLayerGroup::background() const
{
    return m_impl->background;
}

void CanvasLayerGroup::setBackground(const QColor &t_value)
{
    m_impl->background = t_value;
}

bool CanvasLayerGroup::ensureSink(QRhi *rhi, const QSize &size) const
{
    if(m_impl->canvasTexture && m_impl->canvasSize == size)
        return true;

    releaseSink();

    m_impl->canvasTexture = rhi->newTexture(QRhiTexture::RGBA8, size, 1,
                                            QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    if(!m_impl->canvasTexture->create())
    {
        qWarning() << "CanvasLayerGroup: sink texture create failed" << size;
        delete m_impl->canvasTexture;
        m_impl->canvasTexture = nullptr;
        return false;
    }

    QRhiColorAttachment colorAtt(m_impl->canvasTexture);
    QRhiTextureRenderTargetDescription rtDesc(colorAtt);
    m_impl->canvasRT = rhi->newTextureRenderTarget(rtDesc);
    m_impl->canvasRP = m_impl->canvasRT->newCompatibleRenderPassDescriptor();
    m_impl->canvasRT->setRenderPassDescriptor(m_impl->canvasRP);
    if(!m_impl->canvasRT->create())
    {
        qWarning() << "CanvasLayerGroup: sink render target create failed" << size;
        releaseSink();
        return false;
    }

    m_impl->canvasSize = size;
    return true;
}

void CanvasLayerGroup::releaseSink() const
{
    if(!m_impl->canvasRP && !m_impl->canvasRT && !m_impl->canvasTexture)
        return;

    // QRhi resources must be destroyed on the main thread. If we're not on it
    // (group destroyed during worker-thread graph/sequence editing), hand the
    // pointers to the main thread to delete - detached here first, so it's
    // safe even if this CanvasLayerGroup is gone by the time the deletion runs.
    if(QThread::isMainThread())
    {
        delete m_impl->canvasRP;
        delete m_impl->canvasRT;
        delete m_impl->canvasTexture;
    }
    else
    {
        QRhiRenderPassDescriptor *rp = m_impl->canvasRP;
        QRhiTextureRenderTarget *rt = m_impl->canvasRT;
        QRhiTexture *tex = m_impl->canvasTexture;
        QMetaObject::invokeMethod(QCoreApplication::instance(), [rp, rt, tex](){
            delete rp;
            delete rt;
            delete tex;
        }, Qt::QueuedConnection);
    }

    m_impl->canvasRP = nullptr;
    m_impl->canvasRT = nullptr;
    m_impl->canvasTexture = nullptr;
    m_impl->canvasSize = QSize();
}

void CanvasLayerGroup::queueClipForRender(const CanvasClipRenderState &t_state)
{
    QMutexLocker lock(&m_impl->activeClipsMutex);
    m_impl->pendingActiveClips.append(t_state);
}

void CanvasLayerGroup::processChannels(ProcessContext &t_context)
{
    // Runs on either the GUI thread or keira's eval thread (via a
    // SequenceNode) - QRhi is main-thread only, so just flag a render here
    // (mirrors CanvasSubGraphNode::evaluate()); CanvasRenderManager calls
    // renderMainThread() from the main thread when it's due.
    m_impl->needsRender.store(true);

    {
        QMutexLocker lock(&m_impl->activeClipsMutex);
        m_impl->pendingActiveClips.clear();
    }

    // Recurses into sub-layers -> ClipLayer::processChannels() -> each active
    // CanvasClip::processChannels(), which calls queueClipForRender() above.
    LayerGroup::processChannels(t_context);

    {
        QMutexLocker lock(&m_impl->activeClipsMutex);
        m_impl->activeClips = m_impl->pendingActiveClips;
    }
}

bool CanvasLayerGroup::takeNeedsRender() const
{
    return m_impl->needsRender.exchange(false);
}

void CanvasLayerGroup::renderMainThread() const
{
    if(!m_impl->manager)
        return;
    RhiContext *rhiCtx = m_impl->manager->rhiContext();
    if(!rhiCtx || !rhiCtx->isValid())
        return;
    QRhi *rhi = rhiCtx->rhi();

    const QSize size(std::max(1, m_impl->width), std::max(1, m_impl->height));
    if(!ensureSink(rhi, size))
        return;

    QRhiCommandBuffer *cb = nullptr;
    if(rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
        return;

    // Clear the sink to the background colour.
    cb->beginPass(m_impl->canvasRT, m_impl->background, { 1.0f, 0 });
    cb->endPass();

    // Held for the whole compositing loop below (not just a snapshot copy) so
    // a CanvasClip referenced by m_impl->activeClips can't be destroyed out
    // from under it - ~CanvasClip() blocks on this same lock via
    // clipBeingDestroyed() before it tears down its graph.
    QMutexLocker activeClipsLock(&m_impl->activeClipsMutex);

    if(!m_impl->activeClips.isEmpty())
    {
        DMXMatrix matrix;
        RoutineEvaluationContext inner(matrix);
        inner.rhiContext = rhiCtx;
        inner.rhiCommandBuffer = cb;
        inner.canvasResolution = size;

        // Composite each active clip's rendered output onto a running "base"
        // texture, back to front (first sub-layer's clip is the bottommost).
        // base starts as the background-cleared sink itself (safe to sample
        // right after the clear pass, the same way chained canvas producer
        // nodes already sample each other's just-rendered output within one
        // frame). Ping-pong between the two persistent compositors so no
        // composite ever reads and writes the same physical texture.
        RhiTextureData base{m_impl->canvasTexture, size};
        int compositorIndex = 0;

        for(const auto &state : m_impl->activeClips)
        {
            if(!state.clip)
                continue;
            auto *graph = state.clip->contentGraph();
            if(!graph)
                continue;

            graph->drainCommandQueue();
            inner.relativeTime = state.relativeTime;
            inner.globalTime = state.globalTime;
            graph->evaluate(&inner);

            auto *outputNode = dynamic_cast<CanvasOutputNode*>(graph->findNode("Output"));
            const RhiTextureData clipTexture = outputNode ? outputNode->inputTexture() : RhiTextureData{};
            if(!clipTexture.texture)
                continue;

            CanvasCompositeNode *&compositor = m_impl->compositors[compositorIndex % 2];
            if(!compositor)
            {
                compositor = new CanvasCompositeNode;
                compositor->createParameters();
            }
            ++compositorIndex;

            compositor->setValue(CanvasCompositeNode::Base, base);
            compositor->setValue(CanvasCompositeNode::Top, clipTexture);
            compositor->setValue(CanvasCompositeNode::Position, state.position);
            compositor->setValue(CanvasCompositeNode::Scale, state.scale);
            compositor->setValue(CanvasCompositeNode::Rotation, state.rotation);
            compositor->setValue(CanvasCompositeNode::Origin, state.center);
            compositor->setValue(CanvasCompositeNode::Alpha, state.strength);
            compositor->evaluate(&inner);

            base = compositor->findParameter(CanvasCompositeNode::Output)->value().value<RhiTextureData>();
        }

        if(base.texture && base.texture != m_impl->canvasTexture && base.size == size)
        {
            QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
            QRhiTextureCopyDescription copyDesc;
            u->copyTexture(m_impl->canvasTexture, base.texture, copyDesc);
            cb->resourceUpdate(u);
        }
    }

    rhi->endOffscreenFrame();
}

QRhiTexture *CanvasLayerGroup::outputTexture() const
{
    return m_impl->canvasTexture;
}

QSize CanvasLayerGroup::canvasSize() const
{
    return m_impl->canvasSize;
}

void CanvasLayerGroup::addPixelLayout(PixelLayout *t_layout)
{
    if(m_impl->pixelLayouts.contains(t_layout))
        return;
    m_impl->pixelLayouts << t_layout;
    emit pixelLayoutAdded(t_layout);
}

void CanvasLayerGroup::removePixelLayout(PixelLayout *t_layout)
{
    if(m_impl->pixelLayouts.removeOne(t_layout))
        emit pixelLayoutRemoved(t_layout);
}

PixelLayout *CanvasLayerGroup::pixelLayoutAtIndex(int t_index) const
{
    return m_impl->pixelLayouts[t_index];
}

int CanvasLayerGroup::pixelLayoutCount() const
{
    return m_impl->pixelLayouts.length();
}

const QVector<PixelLayout*> &CanvasLayerGroup::pixelLayouts() const
{
    return m_impl->pixelLayouts;
}

QWidget *CanvasLayerGroup::createEditor()
{
    return new CanvasLayerGroupEditor(this);
}

QVector<PixelSource*> CanvasLayerGroup::sources() const
{
    QVector<PixelSource*> results;

    if(m_impl->pixelLayouts.isEmpty())
    {
        auto sources = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[](SceneObject *obj, bool *keepGoing){
            *keepGoing = true;
            return dynamic_cast<PixelSource*>(obj);
        });

        for(auto src : sources)
            results << dynamic_cast<PixelSource*>(src);
    }
    else
    {
        for(auto pixelLayout : m_impl->pixelLayouts)
            results << pixelLayout->sources();
    }

    return results;
}

void CanvasLayerGroup::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    LayerGroup::readFromJson(t_json, t_context);

    m_impl->width = std::max(1, t_json.value("width").toInt(256));
    m_impl->height = std::max(1, t_json.value("height").toInt(256));
    if(t_json.contains("background"))
        m_impl->background = QColor(t_json.value("background").toString());

    if(t_json.contains("pixelLayouts"))
    {
        auto pixelLayoutArray = t_json.value("pixelLayouts").toArray();
        for(auto layoutObj : pixelLayoutArray)
        {
            auto layout = t_context.project->pixelLayouts()->findLayoutWithId(layoutObj.toString().toLatin1());

            if(layout)
                m_impl->pixelLayouts.append(layout);
        }
    }
}

void CanvasLayerGroup::writeToJson(QJsonObject &t_json) const
{
    LayerGroup::writeToJson(t_json);

    t_json.insert("width", m_impl->width);
    t_json.insert("height", m_impl->height);
    t_json.insert("background", m_impl->background.name(QColor::HexArgb));

    QJsonArray pixelLayoutArray;
    for(auto pl : m_impl->pixelLayouts)
    {
        pixelLayoutArray.append(QString{pl->uniqueId()});
    }

    t_json.insert("pixelLayouts", pixelLayoutArray);
}

} // namespace photon

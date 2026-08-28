#include <QPaintEvent>
#include <QPainter>
#include <QPluginLoader>
#include <QRegularExpression>
#include "pluginfactory.h"
#include "photoncore.h"
#include "iplugin.h"
#include "gui/panel/buspanel.h"
#include "gui/panel/sequencepanel.h"
#include "gui/panel/propertiespanel.h"
#include "gui/panel/projectpanel.h"
#include "gui/panel/routineeditpanel.h"
#include "gui/panel/dmxviewerpanel.h"
#include "gui/panel/dmxpatchpanel.h"
#include "gui/panel/canvasviewerpanel.h"
#include "gui/panel/canvaspreviewpanel.h"
#include "gui/panel/surfacepanel.h"
#include "gui/panel/songlibrarypanel.h"

#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"
#include "graph/bus/dmxsubgraphnode.h"
#include "graph/bus/identifyfixturenode.h"
#include "graph/bus/dmxreadernode.h"
#include "graph/bus/sequencenode.h"
#include "graph/bus/surfacenode.h"
#include "routine/node/fixturewriternode.h"
#include "routine/node/globalsnode.h"
#include "routine/node/numberinputnode.h"
#include "routine/node/pointinputnode.h"
#include "routine/node/colorinputnode.h"
#include "routine/node/fixtureinfonode.h"
#include "routine/node/canvaswriternode.h"
#include "routine/node/createtexturenode.h"
#include "graph/node/math/trigonometrynode.h"
#include "graph/node/math/arithmeticnode.h"
#include "graph/node/switchnode.h"
#include "graph/node/math/delaynode.h"
#include "graph/node/writedmxchannelnode.h"
#include "graph/node/gizmo/buttongizmonode.h"
#include "graph/node/gizmo/slidergizmonode.h"
#include "graph/node/gizmo/gizmovaluenode.h"
#include "graph/node/fixture/fixturesubgraphnode.h"
#include "graph/node/fixture/fixturestatenode.h"
#include "graph/node/graphcontextnode.h"
#include "graph/node/fixture/allfixturesnode.h"
#include "graph/node/fixture/selectfixturesnode.h"
#include "graph/node/fixture/fixturegroupnode.h"
#include "graph/node/scene/sceneobjectinfonode.h"
#include "graph/node/scene/matrixdecomposenode.h"
#include "graph/node/pixel/pixelgraph.h"
#include "graph/node/canvas/canvassubgraphnode.h"
#include "graph/node/canvas/canvasoutputnode.h"
#include "graph/node/canvas/canvasfillnode.h"
#include "graph/node/canvas/canvastransformnode.h"
#include "graph/node/canvas/canvasnoisenode.h"
#include "graph/node/canvas/canvaslevelsnode.h"
#include "graph/node/canvas/canvascompositenode.h"
#include "graph/node/canvas/canvasshapenode.h"
#include "graph/node/canvas/canvastilenode.h"
#include "graph/node/canvas/canvasmasknode.h"
#include "graph/node/canvas/canvaswipenode.h"
#include "graph/node/canvas/canvasgradientnode.h"
#include "graph/node/canvas/canvasgradientmapnode.h"
#include "graph/node/canvas/palettetogradientnode.h"
#include "graph/node/canvas/gradientreversenode.h"
#include "graph/node/canvas/gradientremapnode.h"
#include "graph/node/canvas/gradientsamplenode.h"
#include "graph/node/canvas/gradientmixnode.h"

#include "sequence/constantchanneleffect.h"
#include "sequence/gradientchanneleffect.h"
#include "sequence/masterlayerchanneleffect.h"
#include "channel/splinechanneleffect.h"
#include "audio/levelanalysisprocess.h"
#include "audio/virtualdjcaptureprocess.h"

#include "model/parameter/anyparameter.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/buttonparameter.h"
#include "model/parameter/stringparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/colorparameter.h"
#include "graph/parameter/canvasparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "graph/parameter/pathparameter.h"
#include "graph/parameter/vector3dparameter.h"
#include "graph/parameter/matrixparameter.h"
#include "graph/parameter/textureparameter.h"
#include "graph/parameter/point2dparameter.h"
#include "graph/parameter/fixturelistparameter.h"
#include "graph/parameter/pixellistparameter.h"
#include "graph/parameter/tagsparameter.h"
#include "graph/parameter/rhitextureparameter.h"
#include "graph/parameter/pixellayoutlistparameter.h"
#include "graph/parameter/colorpaletteparameter.h"
#include "graph/parameter/gradientparameter.h"

#include "sequence/fixtureclip.h"
#include "sequence/canvasclip.h"

namespace photon {

DefaultPanel::DefaultPanel(const PanelId &panelId) : Panel(panelId, nullptr)
{

}

void DefaultPanel::paintEvent(QPaintEvent *pe)
{
    QPainter p(this);

    p.fillRect(pe->rect(),QColor(150,150,150,255));

    p.drawText(pe->rect(),"Plugin not loaded: " + id());
}

class PluginFactory::Impl
{
public:
    Impl(PluginFactory *_facade);
    ~Impl();
    void init();
    void registerPlugin(QObject *plugin, const QString &path);

    QVector<IPlugin*> plugins;
    QHash<QByteArray, EffectInformation> effects;
    QHash<QByteArray, ClipInformation> clips;
    QHash<QByteArray, AudioProcessorInformation> audioProcessors;
    QHash<PanelId, std::function<Panel*()>> panels;
    keira::NodeLibrary nodeLibrary;

    PluginFactory *facade;
};

PluginFactory::Impl::Impl(PluginFactory *_facade):facade(_facade)
{


}

PluginFactory::Impl::~Impl()
{

}

void PluginFactory::Impl::init()
{


}

void PluginFactory::Impl::registerPlugin(QObject *plugin, const QString &path)
{
    IPlugin *pluginInterface = qobject_cast<IPlugin *>(plugin);
    if(pluginInterface)
    {
        PluginContext pc;
        pc.core = photonApp;
        pc.path = path;

        if(pluginInterface->minimumHostVersion() <= photonApp->version())
        {

            if(pluginInterface->initialize(pc))
            {
                pluginInterface->path = path;
                plugins.append(pluginInterface);
                emit facade->pluginAdded(pluginInterface);
                qDebug() << "Plugin Initialized: " << pluginInterface->name() << "  version: " << pluginInterface->version().toString();
            } else {
                qWarning() << "Plugin Failed to initialize: " << pluginInterface->name() << "  version: " << pluginInterface->version().toString();
            }
        }
        else
        {
            qWarning() << "Plugin " << pluginInterface->name() << "  requires a minimum host version of: " << pluginInterface->minimumHostVersion().toString();
        }
    }
}

PluginFactory::PluginFactory(QObject *parent) : QObject(parent), m_impl(new Impl(this))
{
}

PluginFactory::~PluginFactory()
{

}

void PluginFactory::init()
{
    m_impl->init();

    //registerPluginNode(StylesheetBolt::info());
    registerPluginPanel("photon.project",[](){return new ProjectPanel;});
    registerPluginPanel("photon.bus",[](){return new BusPanel;});
    registerPluginPanel("photon.sequence",[](){return new SequencePanel;});
    registerPluginPanel("photon.surface",[](){return new SurfacePanel;});
    registerPluginPanel("photon.properties",[](){return new PropertiesPanel;});
    registerPluginPanel("photon.routine",[](){return new RoutineEditPanel;});
    registerPluginPanel("photon.dmx-viewer",[](){return new DMXViewerPanel;});
    registerPluginPanel("photon.dmx-patch",[](){return new DMXPatchPanel;});
    registerPluginPanel("photon.canvas-viewer",[](){return new CanvasViewerPanel;});
    registerPluginPanel("photon.canvas-preview",[](){return new CanvasPreviewPanel;});
    registerPluginPanel("photon.song-library",[](){return new SongLibraryPanel;});

    registerNode(FixtureWriterNode::info());
    registerNode(GlobalsNode::info());
    registerNode(TrigonometryNode::info());
    registerNode(ArithmeticNode::info());
    registerNode(DelayNode::info());
    registerNode(SwitchNode::info());
    registerNode(DMXWriterNode::info());
    registerNode(DMXReaderNode::info());
    registerNode(DMXGenerateMatrixNode::info());
    registerNode(SequenceNode::info());
    registerNode(SurfaceNode::info());
    registerNode(DMXSubGraphNode::info());
    registerNode(IdentifyFixtureNode::info());
    registerNode(NumberInputNode::info());
    registerNode(ColorInputNode::info());
    registerNode(PointInputNode::info());
    registerNode(FixtureInfoNode::info());
    registerNode(SceneObjectInfoNode::info());
    registerNode(MatrixDecomposeNode::info());
    registerNode(CanvasWriterNode::info());
    registerNode(CreateTextureNode::info());
    registerNode(WriteDMXChannelNode::info());
    registerNode(ButtonGizmoNode::info());
    registerNode(SliderGizmoNode::info());
    registerNode(GizmoValueNode::info());
    registerNode(FixtureStateNode::info());
    registerNode(FixtureSubGraphNode::info());
    registerNode(GraphContextNode::info());
    // Load-compat: pre-migration graphs serialized their Globals node under a
    // per-domain id; map those ids to the unified GraphContextNode so they still
    // deserialize (the saved params restore the right ports).
    {
        keira::NodeInformation alias([](){ return new GraphContextNode; });
        alias.graphs = QByteArrayList{"__internal"};   // keep out of the Add Node menu
        alias.nodeId = "photon.fixture.globals";
        registerNode(alias);
        alias.nodeId = "photon.canvas.globals";
        registerNode(alias);
        alias.nodeId = "photon.pixel.globals";
        registerNode(alias);
    }
    registerNode(AllFixturesNode::info());
    registerNode(SelectFixturesNode::info());
    registerNode(FixtureGroupNode::info());
    registerNode(PixelGraph::info());
    registerNode(CanvasSubGraphNode::info());
    registerNode(CanvasOutputNode::info());
    registerNode(CanvasFillNode::info());
    registerNode(CanvasTransformNode::info());
    registerNode(CanvasNoiseNode::info());
    registerNode(CanvasLevelsNode::info());
    registerNode(CanvasCompositeNode::info());
    registerNode(CanvasShapeNode::info());
    registerNode(CanvasTileNode::info());
    registerNode(CanvasMaskNode::info());
    registerNode(CanvasWipeNode::info());
    registerNode(CanvasGradientNode::info());
    registerNode(CanvasGradientMapNode::info());
    registerNode(PaletteToGradientNode::info());
    registerNode(GradientReverseNode::info());
    registerNode(GradientRemapNode::info());
    registerNode(GradientSampleNode::info());
    registerNode(GradientMixNode::info());

    registerChannelEffect(ConstantChannelEffect::info());
    registerChannelEffect(GradientChannelEffect::info());
    registerChannelEffect(MasterLayerChannelEffect::info());
    registerChannelEffect(SplineChannelEffect::info());

    registerAudioProcessor(LevelAnalysisProcess::info());
    registerAudioProcessor(VirtualDJCaptureProcess::info());

    registerClip(FixtureClip::info());
    registerClip(CanvasClip::info());

    m_impl->nodeLibrary.registerParameter(keira::AnyParameter::ParameterId,    [](){return new keira::AnyParameter();});
    m_impl->nodeLibrary.registerParameter(keira::DecimalParameter::ParameterId,[](){return new keira::DecimalParameter();});
    m_impl->nodeLibrary.registerParameter(keira::IntegerParameter::ParameterId,[](){return new keira::IntegerParameter();});
    m_impl->nodeLibrary.registerParameter(keira::ButtonParameter::ParameterId,[](){return new keira::ButtonParameter();});
    m_impl->nodeLibrary.registerParameter(keira::StringParameter::ParameterId,[](){return new keira::StringParameter();});
    m_impl->nodeLibrary.registerParameter(keira::OptionParameter::ParameterId,[](){return new keira::OptionParameter();});
    m_impl->nodeLibrary.registerParameter(keira::StringOptionParameter::ParameterId,[](){return new keira::StringOptionParameter();});
    m_impl->nodeLibrary.registerParameter(keira::BooleanParameter::ParameterId,[](){return new keira::BooleanParameter();});
    m_impl->nodeLibrary.registerParameter(DMXMatrixParameter::ParameterId,[](){return new DMXMatrixParameter();});
    m_impl->nodeLibrary.registerParameter(ColorParameter::ParameterId,[](){return new ColorParameter();});
    m_impl->nodeLibrary.registerParameter(CanvasParameter::ParameterId,[](){return new CanvasParameter();});
    m_impl->nodeLibrary.registerParameter(FixtureParameter::ParameterId,[](){return new FixtureParameter();});
    m_impl->nodeLibrary.registerParameter(PathParameter::ParameterId,[](){return new PathParameter();});
    m_impl->nodeLibrary.registerParameter(Vector3DParameter::ParameterId,[](){return new Vector3DParameter();});
    m_impl->nodeLibrary.registerParameter(MatrixParameter::ParameterId,[](){return new MatrixParameter();});
    m_impl->nodeLibrary.registerParameter(TextureParameter::ParameterId,[](){return new TextureParameter();});
    m_impl->nodeLibrary.registerParameter(Point2DParameter::ParameterId,[](){return new Point2DParameter();});
    m_impl->nodeLibrary.registerParameter(FixtureListParameter::ParameterId,[](){return new FixtureListParameter();});
    m_impl->nodeLibrary.registerParameter(ColorPaletteParameter::ParameterId,[](){return new ColorPaletteParameter();});
    m_impl->nodeLibrary.registerParameter(GradientParameter::ParameterId,[](){return new GradientParameter();});
    m_impl->nodeLibrary.registerParameter(PixelListParameter::ParameterId,[](){return new PixelListParameter();});
    m_impl->nodeLibrary.registerParameter(RhiTextureParameter::ParameterId,[](){return new RhiTextureParameter();});
    m_impl->nodeLibrary.registerParameter(PixelLayoutListParameter::ParameterId,[](){return new PixelLayoutListParameter();});
    m_impl->nodeLibrary.registerParameter(TagsParameter::ParameterId,[](){return new TagsParameter();});

    //qDebug() << "Node Count:" << m_impl->nodeHash.size();
}

QVector<IPlugin*> PluginFactory::plugins()
{
    return m_impl->plugins;
}

IPlugin *PluginFactory::pluginWithId(const PluginId &id)
{
    for(auto plugin : m_impl->plugins)
    {
        if(plugin->id() == id)
            return plugin;
    }
    return nullptr;
}

QVector<EffectInformation> PluginFactory::channelEffects() const
{
    return m_impl->effects.values();
}

void PluginFactory::loadPluginsFromFolder(const QDir &dir)
{
    #if defined(Q_OS_WIN)
    QStringList filter("*.dll");
    #elif defined(Q_OS_MAC)
    QStringList filter("*.dylib");
    #endif

    for(const QString &folderName : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QDir pluginDir(dir.path() + "/" + folderName);

        #if defined(Q_OS_WIN)
        QString path = pluginDir.absoluteFilePath(folderName + ".dll");
        #elif defined(Q_OS_MAC)
        QString path = pluginDir.absoluteFilePath(folderName + ".dylib");
        #endif

        QPluginLoader loader(path);
        QObject *plugin = loader.instance();
        if (plugin)
        {
            m_impl->registerPlugin(plugin, pluginDir.path());
        } else {
            qDebug() << path;
            qDebug() << loader.errorString();
            qDebug() << loader.metaData();
        }
        /*
        for(const QString &fileName : pluginDir.entryList(filter, QDir::Files))
        {
            QString path = pluginDir.absoluteFilePath(fileName);
            QPluginLoader loader(path);
            QObject *plugin = loader.instance();
            if (plugin)
            {
                m_impl->registerPlugin(plugin, pluginDir.path());
            } else {
                qDebug() << loader.errorString();
                qDebug() << loader.metaData();
            }
        }
        */
    }
}

void PluginFactory::registerChannelEffect(const EffectInformation &info)
{
    m_impl->effects.insert(info.effectId, info);
}

ChannelEffect *PluginFactory::createChannelEffect(const QByteArray &effectId) const
{
    auto info = m_impl->effects[effectId];
    if(info.effectId == effectId)
    {
        auto effect = info.callback();
        effect->setName(info.name);
        effect->setId(info.effectId);
        return effect;
    }
    return nullptr;
}


void PluginFactory::registerAudioProcessor(const AudioProcessorInformation &info)
{
    m_impl->audioProcessors.insert(info.id, info);
}

QVector<AudioProcessorInformation> PluginFactory::audioProcessors() const
{
    return m_impl->audioProcessors.values();
}

AudioProcessor *PluginFactory::createAudioProcessor(const QByteArray &effectId) const
{
    auto info = m_impl->audioProcessors[effectId];
    if(info.id == effectId)
    {
        auto effect = info.callback();
        return effect;
    }
    return nullptr;
}


void PluginFactory::registerClip(const ClipInformation &info)
{
    m_impl->clips.insert(info.id, info);
}

QVector<ClipInformation> PluginFactory::clips() const
{
    return m_impl->clips.values();
}

Clip *PluginFactory::createClip(const QByteArray &effectId) const
{
    auto info = m_impl->clips[effectId];
    if(info.id == effectId)
    {
        auto effect = info.callback();
        effect->setName(info.name);
        effect->setId(info.id);
        return effect;
    }
    return nullptr;
}

void PluginFactory::registerPluginPanel(const PanelId &panelId, std::function<Panel*()> cb)
{
    m_impl->panels.insert(panelId, cb);
}

keira::NodeLibrary *PluginFactory::nodeLibrary() const
{
    return &m_impl->nodeLibrary;
}

void PluginFactory::registerNode(const keira::NodeInformation nodeInfo)
{
    m_impl->nodeLibrary.registerNode(nodeInfo);
}

keira::Node *PluginFactory::createNode(const QByteArray &nodeId)
{
    return m_impl->nodeLibrary.createNode(nodeId);
}

keira::FolderElement *PluginFactory::createNodeTree(std::function<bool(const keira::NodeInformation &)> pred)
{
    return m_impl->nodeLibrary.createNodeTree(pred);
}

Panel *PluginFactory::createPanel(const PanelId &panelId)
{
    auto factory = m_impl->panels.constFind(panelId);
    if(factory == m_impl->panels.constEnd())
    {
        // Not fatal - DefaultPanel names the missing id in place so it's
        // visible in the UI - but say it out loud too. The usual cause is a
        // saved layout naming a panel whose plugin didn't register under that
        // exact id, which is otherwise easy to misread as the plugin having
        // failed to load.
        qWarning() << "No panel registered for id" << panelId << "- using a placeholder";
        return new DefaultPanel(panelId);
    }

    Panel *panel = (*factory)();

    // saveLayout() persists panel->id() and restoreLayout() feeds that back in
    // here as the lookup key, so a panel whose own id differs from the id it
    // was registered under will save happily and then fail to restore. Catch
    // that at the source rather than leaving it to surface later as a layout
    // that quietly comes back broken.
    if(panel && panel->id() != panelId)
        qWarning() << "Panel registered as" << panelId << "reports its id as" << panel->id()
                   << "- these must match or a saved layout won't restore it";

    return panel;
}

} // namespace exo

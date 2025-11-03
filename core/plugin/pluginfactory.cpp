#include <QPaintEvent>
#include <QPainter>
#include <QPluginLoader>
#include <QRegularExpression>
#include "pluginfactory.h"
#include "photoncore.h"
#include "iplugin.h"
#include "gui/panel/buspanel.h"
#include "gui/panel/sequencepanel.h"
#include "gui/panel/sequencecollectionpanel.h"
#include "gui/panel/routinecollectionpanel.h"
#include "gui/panel/fixturecollectionpanel.h"
#include "gui/panel/canvascollectionpanel.h"
#include "gui/panel/routineeditpanel.h"
#include "gui/panel/dmxviewerpanel.h"
#include "gui/panel/canvasviewerpanel.h"
#include "gui/panel/tagcollectionpanel.h"
#include "gui/panel/pixellayoutcollectionpanel.h"
#include "gui/panel/surfacepanel.h"
#include "gui/panel/surfacecollectionpanel.h"

#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"
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
#include "graph/node/writedmxchannelnode.h"

#include "falloff/constantfalloffeffect.h"
#include "sequence/constantchanneleffect.h"
#include "sequence/gradientchanneleffect.h"
#include "sequence/masterlayerchanneleffect.h"
#include "channel/splinechanneleffect.h"
#include "channel/togglegizmochanneleffect.h"
#include "channel/palettegizmochanneleffect.h"
#include "channel/gizmoretimechanneleffect.h"

#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/buttonparameter.h"
#include "model/parameter/stringparameter.h"
#include "model/parameter/optionparameter.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/colorparameter.h"
#include "graph/parameter/canvasparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "graph/parameter/pathparameter.h"
#include "graph/parameter/vector3dparameter.h"
#include "graph/parameter/textureparameter.h"
#include "graph/parameter/point2dparameter.h"

#include "sequence/fixtureclip.h"
#include "sequence/canvasclip.h"
#include "sequence/canvasroutineclipeffect.h"

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
    QHash<QByteArray, FalloffEffectInformation> falloffEffects;
    QHash<QByteArray, MaskEffectInformation> maskEffects;
    QHash<QByteArray, ClipInformation> clips;
    QHash<QByteArray, ClipEffectInformation> clipEffects;
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
    registerPluginPanel("photon.bus",[](){return new BusPanel;});
    registerPluginPanel("photon.sequence",[](){return new SequencePanel;});
    registerPluginPanel("photon.routine-collection",[](){return new RoutineCollectionPanel;});
    registerPluginPanel("photon.sequence-collection",[](){return new SequenceCollectionPanel;});
    registerPluginPanel("photon.surface-collection",[](){return new SurfaceCollectionPanel;});
    registerPluginPanel("photon.surface",[](){return new SurfacePanel;});
    registerPluginPanel("photon.fixture-collection",[](){return new FixtureCollectionPanel;});
    registerPluginPanel("photon.canvas-collection",[](){return new CanvasCollectionPanel;});
    registerPluginPanel("photon.routine",[](){return new RoutineEditPanel;});
    registerPluginPanel("photon.dmx-viewer",[](){return new DMXViewerPanel;});
    registerPluginPanel("photon.canvas-viewer",[](){return new CanvasViewerPanel;});
    registerPluginPanel("photon.tag-collection",[](){return new TagCollectionPanel;});
    registerPluginPanel("photon.pixellayout-collection",[](){return new PixelLayoutCollectionPanel;});

    registerNode(FixtureWriterNode::info());
    registerNode(GlobalsNode::info());
    registerNode(TrigonometryNode::info());
    registerNode(ArithmeticNode::info());
    registerNode(DMXWriterNode::info());
    registerNode(DMXReaderNode::info());
    registerNode(DMXGenerateMatrixNode::info());
    registerNode(SequenceNode::info());
    registerNode(SurfaceNode::info());
    registerNode(NumberInputNode::info());
    registerNode(ColorInputNode::info());
    registerNode(PointInputNode::info());
    registerNode(FixtureInfoNode::info());
    registerNode(CanvasWriterNode::info());
    registerNode(CreateTextureNode::info());
    registerNode(WriteDMXChannelNode::info());

    registerFalloffEffect(ConstantFalloffEffect::info());

    registerChannelEffect(ConstantChannelEffect::info());
    registerChannelEffect(GradientChannelEffect::info());
    registerChannelEffect(MasterLayerChannelEffect::info());
    registerChannelEffect(SplineChannelEffect::info());
    registerChannelEffect(ToggleGizmoChannelEffect::info());
    registerChannelEffect(GizmoRetimeChannelEffect::info());
    registerChannelEffect(PaletteGizmoChannelEffect::info());

    registerClipEffect(CanvasRoutineClipEffect::info());

    registerClip(FixtureClip::info());
    registerClip(CanvasClip::info());

    m_impl->nodeLibrary.registerParameter(keira::DecimalParameter::ParameterId,[](){return new keira::DecimalParameter();});
    m_impl->nodeLibrary.registerParameter(keira::IntegerParameter::ParameterId,[](){return new keira::IntegerParameter();});
    m_impl->nodeLibrary.registerParameter(keira::ButtonParameter::ParameterId,[](){return new keira::ButtonParameter();});
    m_impl->nodeLibrary.registerParameter(keira::StringParameter::ParameterId,[](){return new keira::StringParameter();});
    m_impl->nodeLibrary.registerParameter(keira::OptionParameter::ParameterId,[](){return new keira::OptionParameter();});
    m_impl->nodeLibrary.registerParameter(DMXMatrixParameter::ParameterId,[](){return new DMXMatrixParameter();});
    m_impl->nodeLibrary.registerParameter(ColorParameter::ParameterId,[](){return new ColorParameter();});
    m_impl->nodeLibrary.registerParameter(CanvasParameter::ParameterId,[](){return new CanvasParameter();});
    m_impl->nodeLibrary.registerParameter(FixtureParameter::ParameterId,[](){return new FixtureParameter();});
    m_impl->nodeLibrary.registerParameter(PathParameter::ParameterId,[](){return new PathParameter();});
    m_impl->nodeLibrary.registerParameter(Vector3DParameter::ParameterId,[](){return new Vector3DParameter();});
    m_impl->nodeLibrary.registerParameter(TextureParameter::ParameterId,[](){return new TextureParameter();});
    m_impl->nodeLibrary.registerParameter(Point2DParameter::ParameterId,[](){return new Point2DParameter();});

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


void PluginFactory::registerFalloffEffect(const FalloffEffectInformation &info)
{
    m_impl->falloffEffects.insert(info.effectId, info);
}

QVector<FalloffEffectInformation> PluginFactory::falloffEffects() const
{
    return m_impl->falloffEffects.values();
}

FalloffEffect *PluginFactory::createFalloffEffect(const QByteArray &effectId) const
{
    auto info = m_impl->falloffEffects[effectId];
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

void PluginFactory::registerClipEffect(const ClipEffectInformation &info)
{
    m_impl->clipEffects.insert(info.id, info);
}

QVector<ClipEffectInformation> PluginFactory::clipEffects() const
{
    return m_impl->clipEffects.values();
}

BaseEffect *PluginFactory::createClipEffect(const QByteArray &effectId) const
{
    auto info = m_impl->clipEffects[effectId];
    if(info.id == effectId)
    {
        auto effect = info.callback();
        effect->init();
        effect->setName(info.name);
        effect->setId(info.id);
        return effect;
    }
    return nullptr;
}

void PluginFactory::registerMaskEffect(const MaskEffectInformation &info)
{
    m_impl->maskEffects.insert(info.effectId, info);
}

QVector<MaskEffectInformation> PluginFactory::maskEffects() const
{
    return m_impl->maskEffects.values();
}

MaskEffect *PluginFactory::createMaskEffect(const QByteArray &effectId) const
{
    auto info = m_impl->maskEffects[effectId];
    if(info.effectId == effectId)
    {
        auto effect = info.callback();
        effect->setName(info.name);
        effect->setId(info.effectId);
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
    return m_impl->panels.value(panelId, [panelId](){
        return new DefaultPanel(panelId);
    })();
}

} // namespace exo

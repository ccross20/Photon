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

#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"
#include "graph/bus/sequencenode.h"
#include "routine/node/fixturewriternode.h"
#include "routine/node/globalsnode.h"
#include "routine/node/numberinputnode.h"
#include "routine/node/colorinputnode.h"
#include "graph/node/math/trigonometrynode.h"
#include "graph/node/math/arithmeticnode.h"

#include "falloff/constantfalloffeffect.h"
#include "sequence/constantchanneleffect.h"
#include "sequence/masterlayerchanneleffect.h"

#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/buttonparameter.h"
#include "model/parameter/stringparameter.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/colorparameter.h"

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
    registerPluginPanel("photon.fixture-collection",[](){return new FixtureCollectionPanel;});
    registerPluginPanel("photon.canvas-collection",[](){return new CanvasCollectionPanel;});
    registerPluginPanel("photon.routine",[](){return new RoutineEditPanel;});
    registerPluginPanel("photon.dmx-viewer",[](){return new DMXViewerPanel;});

    registerNode(FixtureWriterNode::info());
    registerNode(GlobalsNode::info());
    registerNode(TrigonometryNode::info());
    registerNode(ArithmeticNode::info());
    registerNode(DMXWriterNode::info());
    registerNode(DMXGenerateMatrixNode::info());
    registerNode(SequenceNode::info());
    registerNode(NumberInputNode::info());
    registerNode(ColorInputNode::info());

    registerFalloffEffect(ConstantFalloffEffect::info());

    registerChannelEffect(ConstantChannelEffect::info());
    registerChannelEffect(MasterLayerChannelEffect::info());

    m_impl->nodeLibrary.registerParameter(keira::DecimalParameter::ParameterId,[](){return new keira::DecimalParameter();});
    m_impl->nodeLibrary.registerParameter(keira::IntegerParameter::ParameterId,[](){return new keira::IntegerParameter();});
    m_impl->nodeLibrary.registerParameter(keira::ButtonParameter::ParameterId,[](){return new keira::ButtonParameter();});
    m_impl->nodeLibrary.registerParameter(keira::StringParameter::ParameterId,[](){return new keira::StringParameter();});
    m_impl->nodeLibrary.registerParameter(DMXMatrixParameter::ParameterId,[](){return new DMXMatrixParameter();});
    m_impl->nodeLibrary.registerParameter(ColorParameter::ParameterId,[](){return new ColorParameter();});

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

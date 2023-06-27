#ifndef PLUGINFACTORY_H
#define PLUGINFACTORY_H

#include <QObject>
#include <QDir>
#include <QScopedPointer>
#include "photon-global.h"
#include "gui/panel.h"
#include "model/node.h"
#include "library/nodelibrary.h"
#include "sequence/channeleffect.h"
#include "falloff/falloffeffect.h"
#include "fixture/maskeffect.h"
#include "sequence/clip.h"
#include "sequence/clipeffect.h"

namespace photon {

class IPlugin;


class DefaultPanel : public Panel
{
    Q_OBJECT
public:
    DefaultPanel(const PanelId &id);

protected:
    void paintEvent(QPaintEvent *pe) override;
};

class PHOTONCORE_EXPORT PluginFactory : public QObject
{
    Q_OBJECT
public:

    explicit PluginFactory(QObject *parent = nullptr);
    ~PluginFactory();

    void init();
    void loadPluginsFromFolder(const QDir &folder);

    void registerPluginPanel(const PanelId &panelId, std::function<Panel*()> cb);
    void registerChannelEffect(const EffectInformation &info);
    QVector<EffectInformation> channelEffects() const;
    ChannelEffect *createChannelEffect(const QByteArray &effectId) const;


    void registerClip(const ClipInformation &info);
    QVector<ClipInformation> clips() const;
    Clip *createClip(const QByteArray &effectId) const;

    void registerClipEffect(const ClipEffectInformation &info);
    QVector<ClipEffectInformation> clipEffects() const;
    ClipEffect *createClipEffect(const QByteArray &effectId) const;

    void registerFalloffEffect(const FalloffEffectInformation &info);
    QVector<FalloffEffectInformation> falloffEffects() const;
    FalloffEffect *createFalloffEffect(const QByteArray &effectId) const;

    void registerMaskEffect(const MaskEffectInformation &info);
    QVector<MaskEffectInformation> maskEffects() const;
    MaskEffect *createMaskEffect(const QByteArray &effectId) const;

    void registerNode(const keira::NodeInformation nodeInfo);
    keira::NodeLibrary *nodeLibrary() const;
    keira::Node *createNode(const QByteArray &nodeId);
    keira::FolderElement *createNodeTree(std::function<bool(const keira::NodeInformation &)> pred);

    Panel *createPanel(const PanelId &panelId);
    QVector<IPlugin*> plugins();
    IPlugin *pluginWithId(const PluginId &id);


signals:
    void pluginAdded(photon::IPlugin *plugin);

public slots:

private:
    class Impl;
    QScopedPointer<Impl> const m_impl;
};

} // namespace exo

#endif // PLUGINFACTORY_H

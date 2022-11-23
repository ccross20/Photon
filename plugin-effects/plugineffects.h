#ifndef PLUGINEFFECTS_H
#define PLUGINEFFECTS_H

#include <QObject>
#include <QtPlugin>
#include "plugin/iplugin.h"

namespace photon {

class PluginEffects: public QObject, public IPlugin
{
    Q_PLUGIN_METADATA(IID "photon.photon-desktop.IPlugin")
    Q_INTERFACES(photon::IPlugin)
    Q_OBJECT
public:
    bool initialize(const PluginContext &context) override;
    QVersionNumber version() override;
    QVersionNumber minimumHostVersion() override;
    QString name() override;
    QString description() override;
    QString id() override;
};

}// photon namespace

#endif // PLUGINEFFECTS_H

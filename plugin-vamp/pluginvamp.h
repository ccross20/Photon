#ifndef PLUGINVAMP_H
#define PLUGINVAMP_H

#include <QObject>
#include <QtPlugin>
#include "plugin/iplugin.h"
#include "vamp.h"

namespace photon {

class PluginVamp: public QObject, public IPlugin
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
    Vamp vamp;
};

}// photon namespace

#endif // PLUGINVAMP_H

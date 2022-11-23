#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtPlugin>
#include "photoncore.h"
#include "pluginfactory.h"

QT_BEGIN_NAMESPACE
class QString;
class QVersionNumber;
QT_END_NAMESPACE

namespace photon {

struct PluginContext{
    PhotonCore *core;
    QString path;
};

class IPlugin
{
public:

    virtual ~IPlugin() {}
    virtual bool initialize(const PluginContext &context) = 0;
    virtual QVersionNumber version() = 0;
    virtual QVersionNumber minimumHostVersion(){return QVersionNumber(0,0,0);}
    virtual QString name() = 0;
    virtual QString description() = 0;
    virtual QString id() = 0;
    virtual QHash<QString,QString> licenses() const{return QHash<QString,QString>();}
    virtual QString credits(){return QString();}

    QString path;
};

}// Deco namespace

QT_BEGIN_NAMESPACE
//! [3] //! [4]
#define PluginInterface_iid "photon.photon-desktop.IPlugin"

Q_DECLARE_INTERFACE(photon::IPlugin, PluginInterface_iid)
//! [3]
QT_END_NAMESPACE

#endif // IPLUGIN_H

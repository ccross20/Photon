#include <QNetworkInterface>
#include "pluginvamp.h"
#include "plugin/pluginfactory.h"
#include "beatdetectionprocess.h"

namespace photon {

bool PluginVamp::initialize(const PluginContext &context)
{
    //initPluginResource();
    Q_UNUSED(context)
    qDebug() << "Vamp initialized";


    //vamp.printPluginCategoryList();

    vamp.setPluginName("vamp-example-plugins");
    vamp.setPluginId("percussiononsets");
    vamp.setFile(QUrl::fromLocalFile("C:\\Users\\Carter\\Desktop\\click_short.wav"));
    //vamp.start();


    photonApp->plugins()->registerAudioProcessor(BeatDetectionProcess::info());
    //vamp.runPlugin("test","vamp-example-plugins","percussiononsets","out",1,"C:\\Users\\Carter\\Desktop\\click_short.wav","outtest",true);
    //vamp.runPlugin("test","vamp-example-plugins","fixedtempo","out",1,"C:\\Users\\Carter\\Downloads\\01 - Running On Empty.mp3","outtest",true);


    return true;
}

QVersionNumber PluginVamp::version()
{
    return QVersionNumber(0,0,1);
}

QVersionNumber PluginVamp::minimumHostVersion()
{
    return QVersionNumber(0,0,1);
}

QString PluginVamp::name()
{
    return "Vamp";
}

QString PluginVamp::description()
{
    return "Adds audio processing features";
}

QString PluginVamp::id()
{
    return "photon.vamp";
}


} // photon

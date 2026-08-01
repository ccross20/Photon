#include <QNetworkInterface>
#include <QCoreApplication>
#include <QDir>
#include <algorithm>
#include "pluginvamp.h"
#include "vamp.h"
#include "plugin/pluginfactory.h"
#include "beatdetectionprocess.h"

namespace photon {

namespace {
    // Vamp discovers plugins via VAMP_PATH (or a platform default when that's
    // unset). Our bundled plugins (e.g. vamp-example-plugins) ship next to the
    // executable, but VAMP_PATH is developer/machine-specific and easily stale or
    // absent - which silently breaks analysis rather than failing to build. Ensure
    // our own directory is always searched, on top of whatever Vamp would otherwise
    // use, so this doesn't depend on external environment configuration.
    void ensureVampPluginPath()
    {
        std::vector<std::string> path = PluginHostAdapter::getPluginPath();

        const std::string appDir = QCoreApplication::applicationDirPath().toStdString();
        if(std::find(path.begin(), path.end(), appDir) == path.end())
            path.insert(path.begin(), appDir);

        QString joined;
        for(size_t i = 0; i < path.size(); ++i)
        {
            if(i)
                joined += QDir::listSeparator();
            joined += QString::fromStdString(path[i]);
        }

        qputenv("VAMP_PATH", joined.toLocal8Bit());
    }
}

bool PluginVamp::initialize(const PluginContext &context)
{
    Q_UNUSED(context)

    ensureVampPluginPath();

    photonApp->plugins()->registerAudioProcessor(BeatDetectionProcess::info());

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

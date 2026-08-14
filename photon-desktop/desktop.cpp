#include <QDebug>
#include <QApplication>
#include <memory>
#include <map>
#include "photoncore.h"
#include "gui/guimanager.h"


int main(int argc, char *argv[])
{

    qSetMessagePattern("%{function} [%{line}] %{message}");

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    //QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    // Must be set before the QApplication is constructed: on macOS the
    // requested core profile only takes effect if the default format is in
    // place before the platform integration initializes.
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    photon::PhotonCore w(argc, argv);

    w.init();

    w.gui()->launchInterface();

    return w.exec();
}

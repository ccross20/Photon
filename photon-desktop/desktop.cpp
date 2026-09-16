#include <QDebug>
#include <QApplication>
#include <QIcon>
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

    // The bundle's Info.plist (see CMakeLists.txt) is what Finder/Dock read
    // before launch; this is what actually drives the Dock icon when running
    // the bare, non-bundled binary directly (Qt Creator's run button, ninja
    // + launching straight from build/), since there's no bundle for the OS
    // to read an icon from in that case.
    w.setWindowIcon(QIcon(":/icon.png"));

    w.init();

    w.gui()->launchInterface();

    return w.exec();
}

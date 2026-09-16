#include <QDebug>
#include <QApplication>
#include <QSurfaceFormat>
#include <QIcon>
#include <memory>
#include <map>
#include "photoncore.h"
#include "gui/guimanager.h"


int main(int argc, char *argv[])
{

    qSetMessagePattern("%{function} [%{line}] %{message}");

    //QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

#if !defined(Q_OS_MACOS)
    // OpenGL-backend platforms only. macOS runs every QRhi on Metal (see
    // core/rhi/rhibackend.h), where the graphics device is shared explicitly
    // and neither the global GL share group nor a default GL surface format
    // means anything.
    //
    // The share group is what lets the canvas device's textures be imported by
    // the preview window's separate device, so it has to be set before the
    // QApplication is constructed - as does the format: the requested core
    // profile only takes effect if the default is in place before the platform
    // integration initializes.
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);
#endif

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

#include <QDebug>
#include <QApplication>
#include <memory>
#include <map>
#include "photoncore.h"
#include "gui/guimanager.h"


int main(int argc, char *argv[])
{

    qSetMessagePattern("%{function} [%{line}] %{message}");

    //QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    //QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    photon::PhotonCore w(argc, argv);

    w.init();

    w.gui()->launchInterface();

    return w.exec();
}

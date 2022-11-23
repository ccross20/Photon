#include <QApplication>
#include "keira-global.h"
#include "playgroundwidget.h"


int main(int argc, char *argv[])
{
    qSetMessagePattern("%{function} [%{line}] %{message}");

    QApplication a(argc, argv);
    keira::PlaygroundWidget w;
    w.show();
    return a.exec();
}

#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Window : public QMainWindow
{
    Q_OBJECT
public:
    Window(QWidget *parent = nullptr);
    ~Window() override;

signals:
    void closeWindow(Window *window);

public slots:

private slots:

protected:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;


private:
};

} // namespace exo

#endif // WINDOW_H

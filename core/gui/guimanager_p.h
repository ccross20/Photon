#ifndef GUIMANAGER_P_H
#define GUIMANAGER_P_H


#include "guimanager.h"
#include <QDir>
#include <QSettings>
#include <QVector>
#include "window.h"
#include "third-party/advanced-docking/DockManager.h"

namespace photon {

class GuiManager::Impl : public QObject
{
    Q_OBJECT
public:
    Impl(GuiManager *manager);
    ~Impl();

    Panel *createPanel(const PanelId &panelId);
    void addPanel(Panel *panel);

    void launchInterface();
    void createAppWindow();
    void activatePanel(Panel *panel);
    void reloadPanel(Panel *panel);
    void setMainWindowTitle(const QString &title);
    void setProjectModified(bool value);
    void updateMainWindowTitle();
    void clearActivePanel(){m_activePanel = nullptr;}
    Panel *activePanel() const{return m_activePanel;}

    QList<Panel *> panels;
    QString stylesheet;
    QMenuBar *menubar() const;
    ads::CDockManager *dockManager() const{return m_dockManager;}
    QWidget *popOverWidget = nullptr;
    QString mainWindowTitle;
    Window *window;
    bool projectIsModified = false;
    bool quitStarted = false;
    bool closingInterface = false;

protected:
    GuiManager *m_ext;

private slots:

    void dockWidgetRemoved(ads::CDockWidget* DockWidget);
    void dockWidgetAboutToBeRemoved(ads::CDockWidget* DockWidget);
    void floatingWidgetCreated(ads::CFloatingDockContainer* FloatingWidget);

private:
    Panel *m_activePanel;
    QMenuBar *m_macMenubar;
    ads::CDockManager* m_dockManager;


};

} // Deco namespace

#endif // GUIMANAGER_P_H

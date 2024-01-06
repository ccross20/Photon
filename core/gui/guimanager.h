#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include <QObject>
#include "photon-global.h"

class QMenuBar;
class QLayout;

namespace photon {

class Toolbar;

class PHOTONCORE_EXPORT GuiManager : public QObject
{
    Q_OBJECT
public:
    enum DockWidgetArea
    {
        NoDockWidgetArea = 0x00,
        LeftDockWidgetArea = 0x01,
        RightDockWidgetArea = 0x02,
        TopDockWidgetArea = 0x04,
        BottomDockWidgetArea = 0x08,
        CenterDockWidgetArea = 0x10,

        InvalidDockWidgetArea = NoDockWidgetArea,
        OuterDockAreas = TopDockWidgetArea | LeftDockWidgetArea | RightDockWidgetArea | BottomDockWidgetArea,
        AllDockAreas = OuterDockAreas | CenterDockWidgetArea
    };

    GuiManager(QObject *parent = nullptr);
    ~GuiManager() override;

    void init();

    Panel *createPanel(const PanelId &panelId);
    void insertPanel(Panel *panel, const uint windowIndex = 0);
    void insertPanel(const PanelId &panelId, const uint windowIndex = 0);
    Panel *createFloatingPanel(const PanelId &panelId);
    Panel *createDockedPanel(const PanelId &panelId, DockWidgetArea t_area = RightDockWidgetArea, bool t_isTab = false);
    Panel *findPanel(const PanelId &panelId) const;
    void bringPanelToFront(Panel *panel) const;
    int panelCount() const;
    bool saveLayout(const QString &filename = QString());
    bool restoreLayout(const QString &filename = QString());
    Panel * activePanel() const;

    void launchInterface();
    void destroyInterface();
    QMenuBar *menubar() const;

    void showPopoverWidget(QWidget *widget);
    void clearPopoverWidget();
    void setIsLocked(bool value);
    bool isLocked() const;
    void setIsObscured(bool value);
    bool isObscured() const;
    void obscureAllExcept(const QByteArrayList &panelIds);

    static void clearLayout(QLayout *layout);

    class Impl;
signals:
    void focusChanged(const photon::Panel * panel);
    void guiDidInit();
    void guiWillClose();
    void guiDidClose();

public slots:

private slots:
    void panelDestroyed(QObject *obj);
    void popoverWasClosed();
    void projectDidOpen(photon::Project*);
    void projectDidClose();
    void projectWillClose(photon::Project*);
    void activationChanged();
    void prepareForQuit();

private:

    QScopedPointer<Impl> const m_impl;
    friend class Panel;
    friend class PanelDock;

};

} // namespace exo

#endif // GUIMANAGER_H

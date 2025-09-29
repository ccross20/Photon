#ifndef PHOTON_PHOTONCORE_H
#define PHOTON_PHOTONCORE_H

#include <QApplication>
#include <QDir>
#include <QVersionNumber>
#include <QOffscreenSurface>
#include "photon-global.h"

namespace photon {


class PHOTONCORE_EXPORT PhotonCore : public QApplication
{
Q_OBJECT
public:
    PhotonCore(int &argc, char **argv);
    ~PhotonCore();

    void init();
    GuiManager *gui() const;
    PluginFactory *plugins() const;
    Settings *settings() const;
    ResourceManager *resources() const;
    void setProject(Project *);
    Project *project() const;
    void editRoutine(Routine *);

    BusEvaluator *busEvaluator() const;
    Timekeeper *timekeeper() const;

    SequenceCollection *sequences() const;
    SurfaceCollection *surfaces() const;
    QOffscreenSurface *surface() const;
    void initSurface();
    void initOpenGLResources();
    OpenGLResources *openGLResources() const;


    QString appDataPath() const;
    QVersionNumber version() const;

    static QDir pluginDirectory();

public slots:
    void closeProject();
    void loadProject(const QString &path = QString{});
    void newProject();
    void loadSequence(const QString &path = QString{});
    void newSequence();
    void reloadLastSession();

signals:
    void projectWillOpen();
    void projectDidOpen(photon::Project*);
    void projectWillClose(photon::Project*);
    void projectDidClose();

private slots:
    void resourceAdded(const photon::Resource &resource);
    void settingsChanged();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PHOTONCORE_H

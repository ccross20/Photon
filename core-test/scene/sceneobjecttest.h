#ifndef PHOTON_SCENEOBJECTTEST_H
#define PHOTON_SCENEOBJECTTEST_H

#include <QObject>

namespace photon {

// Regression coverage for the reparenting signal-relay bookkeeping in
// SceneObject::Impl::addChild()/removeChild().
class SceneObjectTest : public QObject
{
    Q_OBJECT
public:
    explicit SceneObjectTest(QObject *parent = nullptr);

private slots:
    void reparentingDoesNotLeakSignalRelays();
};

} // namespace photon

#endif // PHOTON_SCENEOBJECTTEST_H

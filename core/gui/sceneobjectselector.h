#ifndef PHOTON_SCENEOBJECTSELECTOR_H
#define PHOTON_SCENEOBJECTSELECTOR_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SceneObjectSelector : public QWidget
{
    Q_OBJECT
public:
    explicit SceneObjectSelector(SceneObject *root, QWidget *parent = nullptr);
    ~SceneObjectSelector();

    void setSelectedObjects(const QVector<SceneObject *> &);
    QVector<SceneObject *> selectedObjects() const;

signals:
    void selectionChanged();

private:

    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SCENEOBJECTSELECTOR_H

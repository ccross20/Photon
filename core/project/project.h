#ifndef PHOTON_PROJECT_H
#define PHOTON_PROJECT_H

#include "photon-global.h"

namespace photon {

class SceneManager;

class PHOTONCORE_EXPORT Project : public QObject
{
    Q_OBJECT
public:
    explicit Project(QObject *parent = nullptr);
    ~Project();

    void addSequence(Sequence *);
    FixtureCollection *fixtures() const;
    RoutineCollection *routines() const;
    SequenceCollection *sequences() const;
    CanvasCollection *canvases() const;
    BusGraph *bus() const;
    SceneObject *sceneRoot() const;
    SceneManager *scene() const;

    void save(const QString &path = QString{}) const;
    void load(const QString &path = QString{});
    void restore(Project &);
    void readFromJson(const QJsonObject &json);
    void writeToJson(QJsonObject &json) const;
signals:

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROJECT_H

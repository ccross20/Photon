#ifndef PHOTON_FIXTUREMODEL_H
#define PHOTON_FIXTUREMODEL_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class Entity;
class ModelModifier;

class FixtureModel : public QObject
{
    Q_OBJECT
public:
    explicit FixtureModel(Fixture *fixture, QObject *parent = nullptr);


    Entity *entity() const;
    Fixture *fixture() const;

    void updateFromDMX(const DMXMatrix &);

signals:

private slots:
    void transformUpdated();

private:
    void recursiveAddModifiers(Entity *);
    QVector<ModelModifier*> m_modifiers;
    Fixture *m_fixture;
    Entity *m_entity;

};

} // namespace photon

#endif // PHOTON_FIXTUREMODEL_H

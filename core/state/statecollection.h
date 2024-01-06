#ifndef PHOTON_STATECOLLECTION_H
#define PHOTON_STATECOLLECTION_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT StateCollection : public QObject
{
    Q_OBJECT
public:

    explicit StateCollection(QObject *parent = nullptr);
    ~StateCollection();

    void clear();
    int stateCount() const;
    State *stateAtIndex(uint) const;
    State *findStateById(const QByteArray &) const;
    const QVector<State*> &states() const;

signals:
    void stateWillBeAdded(photon::State *, int);
    void stateWasAdded(photon::State *, int);
    void stateWillBeRemoved(photon::State *, int);
    void stateWasRemoved(photon::State *, int);

public slots:
    void addState(photon::State *);
    void removeState(photon::State *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_STATECOLLECTION_H

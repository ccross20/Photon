#ifndef PHOTON_ROUTINECOLLECTION_H
#define PHOTON_ROUTINECOLLECTION_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT RoutineCollection : public QObject
{
    Q_OBJECT
public:
    explicit RoutineCollection(QObject *parent = nullptr);
    ~RoutineCollection();


    Routine *findByUniqueId(const QByteArray &id);
    int routineCount() const;
    Routine *routineAtIndex(uint) const;
    const QVector<Routine*> &routines() const;

signals:
    void routineWillBeAdded(photon::Routine *, int);
    void routineWasAdded(photon::Routine *);
    void routineWillBeRemoved(photon::Routine *, int);
    void routineWasRemoved(photon::Routine *);

public slots:
    void addRoutine(photon::Routine *);
    void removeRoutine(photon::Routine *);

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_ROUTINECOLLECTION_H

#include "routinecollection.h"
#include "routine.h"

namespace photon {

class RoutineCollection::Impl
{
public:
    Impl(RoutineCollection *);

    QVector<Routine *> routines;
    RoutineCollection *facade;
};

RoutineCollection::Impl::Impl(RoutineCollection *t_facade):facade(t_facade)
{

}

RoutineCollection::RoutineCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

RoutineCollection::~RoutineCollection()
{
    for(auto sequence : m_impl->routines)
        delete sequence;
    delete m_impl;
}

Routine *RoutineCollection::findByUniqueId(const QByteArray &t_id)
{
    auto result = std::find_if(m_impl->routines.cbegin(), m_impl->routines.cend(),[t_id](Routine *t_testRoutine){
                     return t_testRoutine->uniqueId() == t_id;
                 });

    if(result != m_impl->routines.cend())
        return *result;
    return nullptr;
}

const QVector<Routine*> &RoutineCollection::routines() const
{
    return m_impl->routines;
}

int RoutineCollection::routineCount() const
{
    return m_impl->routines.length();
}

Routine *RoutineCollection::routineAtIndex(uint t_index) const
{
    return m_impl->routines.at(t_index);
}

void RoutineCollection::addRoutine(photon::Routine *t_routine)
{
    if(m_impl->routines.contains(t_routine))
        return;
    emit routineWillBeAdded(t_routine, m_impl->routines.length());
    m_impl->routines.append(t_routine);
    emit routineWasAdded(t_routine);
}

void RoutineCollection::removeRoutine(photon::Routine *t_routine)
{
    if(!m_impl->routines.contains(t_routine))
        return;

    int index = 0;
    for(auto seq : m_impl->routines)
    {
        if(seq == t_routine)
            break;
        ++index;
    }

    emit routineWillBeRemoved(t_routine, index);
    m_impl->routines.removeOne(t_routine);
    emit routineWasRemoved(t_routine);
}


} // namespace photon

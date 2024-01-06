#include "statecollection.h"
#include "state.h"


namespace photon {

class StateCollection::Impl
{
public:
    Impl(StateCollection *);

    QVector<State *> states;
    StateCollection *facade;
};

StateCollection::Impl::Impl(StateCollection *t_facade):facade(t_facade)
{

}

StateCollection::StateCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

StateCollection::~StateCollection()
{
    for(auto state : m_impl->states)
        delete state;
    delete m_impl;
}

void StateCollection::clear()
{
    for(auto state : m_impl->states)
        delete state;
    m_impl->states.clear();
}

const QVector<State*> &StateCollection::states() const
{
    return m_impl->states;
}

int StateCollection::stateCount() const
{
    return m_impl->states.length();
}

State *StateCollection::findStateById(const QByteArray &t_id) const
{
    for(auto state : m_impl->states)
        if(state->uniqueId() == t_id)
            return state;

    return nullptr;
}

State *StateCollection::stateAtIndex(uint t_index) const
{
    return m_impl->states.at(t_index);
}

void StateCollection::addState(photon::State *t_state)
{
    if(m_impl->states.contains(t_state))
        return;
    emit stateWillBeAdded(t_state, m_impl->states.length());
    m_impl->states.append(t_state);
    emit stateWasAdded(t_state, m_impl->states.length()-1);
}

void StateCollection::removeState(photon::State *t_state)
{
    if(!m_impl->states.contains(t_state))
        return;

    int index = 0;
    for(auto seq : m_impl->states)
    {
        if(seq == t_state)
            break;
        ++index;
    }

    emit stateWillBeRemoved(t_state, index);
    m_impl->states.removeOne(t_state);
    emit stateWasRemoved(t_state, index);
}

} // namespace photon

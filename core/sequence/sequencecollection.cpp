#include "sequencecollection.h"
#include "sequence.h"

namespace photon {

class SequenceCollection::Impl
{
public:
    Impl(SequenceCollection *);

    QVector<Sequence *> sequences;
    SequenceCollection *facade;
};

SequenceCollection::Impl::Impl(SequenceCollection *t_facade):facade(t_facade)
{

}

SequenceCollection::SequenceCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

SequenceCollection::~SequenceCollection()
{
    for(auto sequence : m_impl->sequences)
        delete sequence;
    delete m_impl;
}

const QVector<Sequence*> &SequenceCollection::sequences() const
{
    return m_impl->sequences;
}

int SequenceCollection::sequenceCount() const
{
    return m_impl->sequences.length();
}

Sequence *SequenceCollection::sequenceAtIndex(uint t_index) const
{
    return m_impl->sequences.at(t_index);
}

void SequenceCollection::addSequence(photon::Sequence *t_sequence)
{
    if(m_impl->sequences.contains(t_sequence))
        return;
    emit sequenceWillBeAdded(t_sequence, m_impl->sequences.length());
    m_impl->sequences.append(t_sequence);
    emit sequenceWasAdded(t_sequence, m_impl->sequences.length()-1);
}

void SequenceCollection::removeSequence(photon::Sequence *t_sequence)
{
    if(!m_impl->sequences.contains(t_sequence))
        return;

    int index = 0;
    for(auto seq : m_impl->sequences)
    {
        if(seq == t_sequence)
            break;
        ++index;
    }

    emit sequenceWillBeRemoved(t_sequence, index);
    m_impl->sequences.removeOne(t_sequence);
    emit sequenceWasRemoved(t_sequence, index);
}

} // namespace photon

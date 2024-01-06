#include "sequencecollection.h"
#include "sequence.h"
#include "gui/panel/sequencepanel.h"
#include "photoncore.h"

namespace photon {

class SequenceCollection::Impl
{
public:
    Impl(SequenceCollection *);

    QHash<Sequence*,SequencePanel*> panels;
    QVector<Sequence *> sequences;
    SequenceCollection *facade;
    Sequence *activeSequence = nullptr;
    SequencePanel *activePanel = nullptr;
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

Sequence *SequenceCollection::activeSequence() const
{
    return m_impl->activeSequence;
}

void SequenceCollection::setActiveSequencePanel(SequencePanel *panel)
{
    m_impl->activePanel = panel;

    if(panel)
        photonApp->gui()->bringPanelToFront(panel);
}

SequencePanel *SequenceCollection::activeSequencePanel() const
{
    return m_impl->activePanel;
}

void SequenceCollection::editSequence(Sequence *t_sequence)
{
    m_impl->activeSequence = t_sequence;
    if(m_impl->panels.contains(t_sequence))
    {
        setActiveSequencePanel(m_impl->panels.value(t_sequence));
    }
    else
    {
        SequencePanel *sequencePanel = static_cast<SequencePanel*>(photonApp->gui()->createDockedPanel("photon.sequence", GuiManager::CenterDockWidgetArea, true));
        sequencePanel->setSequence(t_sequence);
        sequencePanel->setName(t_sequence->name());
        m_impl->panels.insert(t_sequence, sequencePanel);
        connect(sequencePanel, &QObject::destroyed,this, &SequenceCollection::panelDestroyed);

        setActiveSequencePanel(sequencePanel);
    }
}

void SequenceCollection::panelDestroyed(QObject *t_object)
{
    for(auto it = m_impl->panels.constBegin(); it != m_impl->panels.constEnd(); ++it)
    {
        if(it.value() == static_cast<SequencePanel*>(t_object))
        {
            removeSequence(it.key());
            break;
        }
    }

}

void SequenceCollection::clear()
{
    for(auto sequence : m_impl->sequences)
        delete sequence;

    m_impl->sequences.clear();
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

    m_impl->panels.remove(t_sequence);

    emit sequenceWillBeRemoved(t_sequence, index);
    m_impl->sequences.removeOne(t_sequence);
    emit sequenceWasRemoved(t_sequence, index);
}

} // namespace photon

#include "pixellayoutcollection.h"
#include "pixellayout.h"

namespace photon {

class PixelLayoutCollection::Impl
{
public:
    Impl(PixelLayoutCollection *);

    QVector<PixelLayout *> layouts;
    PixelLayoutCollection *facade;
};

PixelLayoutCollection::Impl::Impl(PixelLayoutCollection *t_facade):facade(t_facade)
{

}

PixelLayoutCollection::PixelLayoutCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

PixelLayoutCollection::~PixelLayoutCollection()
{
    for(auto canvas : m_impl->layouts)
        delete canvas;
    delete m_impl;
}

const QVector<PixelLayout*> &PixelLayoutCollection::layouts() const
{
    return m_impl->layouts;
}

int PixelLayoutCollection::layoutCount() const
{
    return m_impl->layouts.length();
}

PixelLayout *PixelLayoutCollection::layoutAtIndex(uint t_index) const
{
    return m_impl->layouts.at(t_index);
}

PixelLayout *PixelLayoutCollection::findLayoutWithId(const QByteArray &t_id) const
{
    for(auto layout : m_impl->layouts)
    {
        if(layout->uniqueId() == t_id)
            return layout;
    }
    return nullptr;
}

void PixelLayoutCollection::addLayout(photon::PixelLayout *t_layout)
{
    if(m_impl->layouts.contains(t_layout))
        return;
    emit layoutWillBeAdded(t_layout, m_impl->layouts.length());
    m_impl->layouts.append(t_layout);
    emit layoutWasAdded(t_layout, m_impl->layouts.length()-1);
}

void PixelLayoutCollection::removeLayout(photon::PixelLayout *t_layout)
{
    if(!m_impl->layouts.contains(t_layout))
        return;

    int index = 0;
    for(auto seq : m_impl->layouts)
    {
        if(seq == t_layout)
            break;
        ++index;
    }

    emit layoutWillBeRemoved(t_layout, index);
    m_impl->layouts.removeOne(t_layout);
    emit layoutWasRemoved(t_layout, index);
}

} // namespace photon

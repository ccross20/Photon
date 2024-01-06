#include "tagcollection.h"

namespace photon {

class TagCollection::Impl
{
public:
    Impl(TagCollection *);

    QStringList tags;
    TagCollection *facade;
};

TagCollection::Impl::Impl(TagCollection *t_facade):facade(t_facade)
{

}

TagCollection::TagCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

TagCollection::~TagCollection()
{
    delete m_impl;
}

void TagCollection::clear()
{
    m_impl->tags.clear();
}

const QStringList &TagCollection::tags() const
{
    return m_impl->tags;
}

int TagCollection::tagCount() const
{
    return m_impl->tags.length();
}

QString TagCollection::tagAtIndex(uint t_index) const
{
    return m_impl->tags.at(t_index);
}

void TagCollection::replaceTags(const QStringList &t_tags)
{
    emit tagsWillBeReplaced();
    auto set = QSet<QString>(t_tags.cbegin(), t_tags.cend());
    m_impl->tags = QStringList{set.cbegin(), set.cend()};
    m_impl->tags.sort();

    emit tagsWereReplaced();
}

void TagCollection::addTag(const QString &t_tag)
{
    if(m_impl->tags.contains(t_tag))
        return;


    int index = 0;
    for(const auto &seq : m_impl->tags)
    {
        if(seq > t_tag)
            break;
        ++index;
    }


    emit tagWillBeAdded(t_tag, index);
    m_impl->tags.insert(index, t_tag);
    emit tagWasAdded(t_tag, index+1);
}

void TagCollection::removeTag(const QString &t_tag)
{
    if(!m_impl->tags.contains(t_tag))
        return;

    int index = 0;
    for(const auto &seq : m_impl->tags)
    {
        if(seq == t_tag)
            break;
        ++index;
    }

    emit tagWillBeRemoved(t_tag, index);
    m_impl->tags.removeOne(t_tag);
    emit tagWasRemoved(t_tag, index);
}

} // namespace photon

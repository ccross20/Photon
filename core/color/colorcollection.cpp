#include "colorcollection.h"
#include "colorresource.h"

namespace photon {

class ColorCollection::Impl
{
public:
    QVector<ColorResource*> colors;
};

ColorCollection::ColorCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl)
{

}

ColorCollection::~ColorCollection()
{
    for(auto color : m_impl->colors)
        delete color;
    delete m_impl;
}

const QVector<ColorResource*> &ColorCollection::colors() const
{
    return m_impl->colors;
}

int ColorCollection::colorCount() const
{
    return m_impl->colors.length();
}

ColorResource *ColorCollection::colorAtIndex(uint t_index) const
{
    return m_impl->colors.at(t_index);
}

ColorResource *ColorCollection::findColorWithId(const QByteArray &t_id) const
{
    for(auto color : m_impl->colors)
    {
        if(color->uniqueId() == t_id)
            return color;
    }
    return nullptr;
}

ColorResource *ColorCollection::findColorWithName(const QString &t_name) const
{
    for(auto color : m_impl->colors)
    {
        if(color->name() == t_name)
            return color;
    }
    return nullptr;
}

void ColorCollection::addColor(photon::ColorResource *t_color)
{
    if(m_impl->colors.contains(t_color))
        return;
    emit colorWillBeAdded(t_color, m_impl->colors.length());
    m_impl->colors.append(t_color);
    emit colorWasAdded(t_color, m_impl->colors.length()-1);
}

void ColorCollection::removeColor(photon::ColorResource *t_color)
{
    if(!m_impl->colors.contains(t_color))
        return;

    int index = 0;
    for(auto color : m_impl->colors)
    {
        if(color == t_color)
            break;
        ++index;
    }

    emit colorWillBeRemoved(t_color, index);
    m_impl->colors.removeOne(t_color);
    emit colorWasRemoved(t_color, index);
}

} // namespace photon

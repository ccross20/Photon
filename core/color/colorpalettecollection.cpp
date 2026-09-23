#include "colorpalettecollection.h"
#include "colorpaletteresource.h"

namespace photon {

class ColorPaletteCollection::Impl
{
public:
    QVector<ColorPaletteResource*> palettes;
};

ColorPaletteCollection::ColorPaletteCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl)
{

}

ColorPaletteCollection::~ColorPaletteCollection()
{
    for(auto palette : m_impl->palettes)
        delete palette;
    delete m_impl;
}

const QVector<ColorPaletteResource*> &ColorPaletteCollection::palettes() const
{
    return m_impl->palettes;
}

int ColorPaletteCollection::paletteCount() const
{
    return m_impl->palettes.length();
}

ColorPaletteResource *ColorPaletteCollection::paletteAtIndex(uint t_index) const
{
    return m_impl->palettes.at(t_index);
}

ColorPaletteResource *ColorPaletteCollection::findPaletteWithId(const QByteArray &t_id) const
{
    for(auto palette : m_impl->palettes)
    {
        if(palette->uniqueId() == t_id)
            return palette;
    }
    return nullptr;
}

ColorPaletteResource *ColorPaletteCollection::findPaletteWithName(const QString &t_name) const
{
    for(auto palette : m_impl->palettes)
    {
        if(palette->name() == t_name)
            return palette;
    }
    return nullptr;
}

void ColorPaletteCollection::addPalette(photon::ColorPaletteResource *t_palette)
{
    if(m_impl->palettes.contains(t_palette))
        return;
    emit paletteWillBeAdded(t_palette, m_impl->palettes.length());
    m_impl->palettes.append(t_palette);
    emit paletteWasAdded(t_palette, m_impl->palettes.length()-1);
}

void ColorPaletteCollection::removePalette(photon::ColorPaletteResource *t_palette)
{
    if(!m_impl->palettes.contains(t_palette))
        return;

    int index = 0;
    for(auto palette : m_impl->palettes)
    {
        if(palette == t_palette)
            break;
        ++index;
    }

    emit paletteWillBeRemoved(t_palette, index);
    m_impl->palettes.removeOne(t_palette);
    emit paletteWasRemoved(t_palette, index);
}

} // namespace photon

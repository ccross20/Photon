#include "pixellayout.h"
#include "pixelsourcelayout.h"

namespace photon {

class PixelLayout::Impl
{
public:
    QVector<PixelSourceLayout*> sources;
    QString name;
    QByteArray uniqueId;
};

PixelLayout::PixelLayout() : m_impl(new Impl)
{

}

PixelLayout::~PixelLayout()
{
    delete m_impl;
}


QByteArray PixelLayout::uniqueId() const
{
    return m_impl->uniqueId;
}


void PixelLayout::addSource(PixelSourceLayout *t_sourceLayout)
{
    emit sourceWillBeAdded(t_sourceLayout,m_impl->sources.length() + 1);
    m_impl->sources.append(t_sourceLayout);
    emit sourceWasAdded(t_sourceLayout,m_impl->sources.length());
    emit layoutModified();
}

void PixelLayout::removeSource(PixelSourceLayout *t_sourceLayout)
{

    emit sourceWillBeRemoved(t_sourceLayout,m_impl->sources.indexOf(t_sourceLayout));
    m_impl->sources.removeOne(t_sourceLayout);
    emit sourceWasRemoved(t_sourceLayout,0);
    emit layoutModified();
}

const QVector<PixelSourceLayout*> &PixelLayout::sourceLayouts() const
{
    return m_impl->sources;
}

QVector<PixelSource*> PixelLayout::sources() const
{
    QVector<PixelSource*> results;

    for(auto source : m_impl->sources)
        results.append(source->source());
    return results;
}

QString PixelLayout::name() const
{
    return m_impl->name;
}

void PixelLayout::setName(const QString &t_name)
{
    m_impl->name = t_name;
}


void PixelLayout::process(CanvasContext &t_context) const
{
    for(auto source : m_impl->sources)
    {
        source->process(t_context);
    }
}

void PixelLayout::readFromJson(const QJsonObject &, const LoadContext &)
{

}

void PixelLayout::writeToJson(QJsonObject &) const
{

}

} // namespace photon

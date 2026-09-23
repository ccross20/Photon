#include "gradientcollection.h"
#include "gradientresource.h"

namespace photon {

class GradientCollection::Impl
{
public:
    QVector<GradientResource*> gradients;
};

GradientCollection::GradientCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl)
{

}

GradientCollection::~GradientCollection()
{
    for(auto gradient : m_impl->gradients)
        delete gradient;
    delete m_impl;
}

const QVector<GradientResource*> &GradientCollection::gradients() const
{
    return m_impl->gradients;
}

int GradientCollection::gradientCount() const
{
    return m_impl->gradients.length();
}

GradientResource *GradientCollection::gradientAtIndex(uint t_index) const
{
    return m_impl->gradients.at(t_index);
}

GradientResource *GradientCollection::findGradientWithId(const QByteArray &t_id) const
{
    for(auto gradient : m_impl->gradients)
    {
        if(gradient->uniqueId() == t_id)
            return gradient;
    }
    return nullptr;
}

GradientResource *GradientCollection::findGradientWithName(const QString &t_name) const
{
    for(auto gradient : m_impl->gradients)
    {
        if(gradient->name() == t_name)
            return gradient;
    }
    return nullptr;
}

void GradientCollection::addGradient(photon::GradientResource *t_gradient)
{
    if(m_impl->gradients.contains(t_gradient))
        return;
    emit gradientWillBeAdded(t_gradient, m_impl->gradients.length());
    m_impl->gradients.append(t_gradient);
    emit gradientWasAdded(t_gradient, m_impl->gradients.length()-1);
}

void GradientCollection::removeGradient(photon::GradientResource *t_gradient)
{
    if(!m_impl->gradients.contains(t_gradient))
        return;

    int index = 0;
    for(auto gradient : m_impl->gradients)
    {
        if(gradient == t_gradient)
            break;
        ++index;
    }

    emit gradientWillBeRemoved(t_gradient, index);
    m_impl->gradients.removeOne(t_gradient);
    emit gradientWasRemoved(t_gradient, index);
}

} // namespace photon

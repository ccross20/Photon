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
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
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


void PixelLayout::process(ProcessContext &t_context) const
{
    for(auto source : m_impl->sources)
    {
        source->process(t_context);
    }
}

void PixelLayout::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    if(t_json.contains("sources"))
    {
        m_impl->sources.clear();
        auto sourceArray = t_json.value("sources").toArray();
        for(auto source : sourceArray)
        {
            auto sourceObj = source.toObject();
            auto newLayout = new PixelSourceLayout();
            newLayout->readFromJson(sourceObj, t_context);

            m_impl->sources.append(newLayout);
        }
        m_impl->name = t_json.value("name").toString();
        m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    }
}

void PixelLayout::writeToJson(QJsonObject &t_json) const
{
    QJsonArray sourceArray;
    for(auto source : m_impl->sources)
    {
        QJsonObject sourceObj;
        source->writeToJson(sourceObj);
        sourceArray.append(sourceObj);
    }
    t_json.insert("sources", sourceArray);
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString{m_impl->uniqueId});
}

} // namespace photon

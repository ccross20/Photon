#include "pixellayout.h"
#include "pixelsourcelayout.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <QVBoxLayout>
#include "gui/resourceeditorwidget.h"
#include "gui/pixellayouteditor.h"

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

QWidget *PixelLayout::createResourceEditor()
{
    // Compact enough to live inline in the Properties panel - name/tags on
    // top, the layout canvas below - rather than behind a separate "Open"
    // step or a floating window.
    auto *container = new QWidget;
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new ResourceEditorWidget(this));
    layout->addWidget(new PixelLayoutEditor(this));

    return container;
}

QString PixelLayout::name() const
{
    return m_impl->name;
}

void PixelLayout::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    notifyResourceChanged();
}


void PixelLayout::process(ProcessContext &t_context, double t_blend) const
{
    for(auto source : m_impl->sources)
    {
        source->process(t_context, t_blend);
    }
}

void PixelLayout::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    // Identity is read unconditionally: it used to sit inside the "sources"
    // guard below, so a layout saved with no sources came back nameless and
    // with a freshly generated id.
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    if(t_json.contains("uniqueId"))
        m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    readResourceJson(t_json);

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
    writeResourceJson(t_json);
}

} // namespace photon

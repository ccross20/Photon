#include <QSize>
#include "canvas.h"

namespace photon {

class Canvas::Impl
{
public:
    QString name;
    QSize size;

};

Canvas::Canvas(QObject *t_parent)
    : QObject{t_parent},m_impl(new Impl)
{

}

Canvas::Canvas(const QString &t_name,  const QSize &t_size, QObject *t_parent):QObject(t_parent),m_impl(new Impl)
{
    m_impl->name = t_name;
    m_impl->size = t_size;
}

Canvas::~Canvas()
{
    delete m_impl;
}

QSize Canvas::size() const
{
    return m_impl->size;
}

void Canvas::setSize(const QSize &t_size)
{
    if(m_impl->size == t_size)
        return;
    m_impl->size = t_size;
    emit metadataUpdated();
}

int Canvas::width() const
{
    return m_impl->size.width();
}

int Canvas::height() const
{
    return m_impl->size.height();
}

QString Canvas::name() const
{
    return m_impl->name;
}

void Canvas::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    emit metadataUpdated();
}


void Canvas::paint(const QImage &t_image)
{
    emit didPaint(t_image);
}

void Canvas::restore(Project &)
{

}

void Canvas::readFromJson(const QJsonObject &t_json, const LoadContext &)
{
    m_impl->name = t_json["name"].toString();

    if(t_json.contains("size"))
    {
        auto sizeObj = t_json.value("size").toObject();

        m_impl->size.setWidth(std::max(std::min(sizeObj.value("width").toInt(1),10000),1));
        m_impl->size.setHeight(std::max(std::min(sizeObj.value("height").toInt(1),10000),1));

    }
}

void Canvas::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name",m_impl->name);

    QJsonObject sizeObj;
    sizeObj.insert("width", m_impl->size.width());
    sizeObj.insert("height", m_impl->size.height());
    t_json.insert("size",sizeObj);
}


} // namespace photon

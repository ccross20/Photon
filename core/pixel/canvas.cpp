#include <QSize>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include "canvas.h"
#include "photoncore.h"
#include "opengl/opengltexture.h"

namespace photon {

class Canvas::Impl
{
public:
    QString name;
    QSize size;
    OpenGLTexture *texture = nullptr;

};

Canvas::Canvas(QObject *t_parent)
    : QObject{t_parent},m_impl(new Impl)
{

}

Canvas::Canvas(const QString &t_name,  const QSize &t_size, QObject *t_parent):QObject(t_parent),m_impl(new Impl)
{
    m_impl->name = t_name;
    m_impl->size = t_size;

    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;
    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);

    m_impl->texture = new OpenGLTexture(true);
    m_impl->texture->resize(&context, QImage::Format::Format_ARGB32_Premultiplied ,t_size.width(), t_size.height());

    context.doneCurrent();
}

Canvas::~Canvas()
{
    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;
    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);
    delete m_impl->texture;
    context.doneCurrent();

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

    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;
    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);
    m_impl->texture->resize(t_size.width(), t_size.height());

    context.doneCurrent();

    emit sizeUpdated(m_impl->size);
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

void Canvas::updateTexture()
{
    emit textureDidUpdate();
}

OpenGLTexture *Canvas::texture() const
{
    return m_impl->texture;
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

    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;
    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);

    qDebug() << m_impl->size;

    m_impl->texture = new OpenGLTexture(true);
    m_impl->texture->resize(&context, QImage::Format::Format_ARGB32_Premultiplied ,m_impl->size.width(), m_impl->size.height());

    context.doneCurrent();
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

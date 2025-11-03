#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include "surface_p.h"
#include "state/stateevaluationcontext.h"
#include "state/state.h"
#include "project/project.h"
#include "photoncore.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixture.h"
#include "surfacegizmocontainer.h"

namespace photon {

Surface::Impl::Impl(Surface *t_facade):facade(t_facade)
{
    uniqueId = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);

}


Surface::Surface(const QString &t_name, QObject *parent)
    : QObject{parent},m_impl(new Impl(this))
{
    m_impl->name = t_name;
}

Surface::~Surface()
{
    delete m_impl;
}

void Surface::init()
{

}

QByteArray Surface::uniqueId() const
{
    return m_impl->uniqueId;
}

const QVector<SurfaceGizmoContainer*> &Surface::containers() const
{
    return m_impl->gizmos;
}

Project *Surface::project() const
{
    return photonApp->project();
}

QString Surface::name() const
{
    return m_impl->name;
}

void Surface::setName(const QString &t_value)
{
    m_impl->name = t_value;
}

void Surface::addGizmoContainer(photon::SurfaceGizmoContainer *t_gizmo)
{
    if(m_impl->gizmos.contains(t_gizmo))
        return;
    emit gizmoContainerWillBeAdded(t_gizmo, m_impl->gizmos.length());
    m_impl->gizmos.append(t_gizmo);

    connect(t_gizmo, &SurfaceGizmoContainer::gizmoWasAdded,this, &Surface::surfaceWasModified);

    emit gizmoContainerWasAdded(t_gizmo, m_impl->gizmos.length()-1);
    emit surfaceWasModified();
}

void Surface::removeGizmoContainer(photon::SurfaceGizmoContainer *t_gizmo)
{
    if(!m_impl->gizmos.contains(t_gizmo))
        return;

    int index = 0;
    for(auto seq : m_impl->gizmos)
    {
        if(seq == t_gizmo)
            break;
        ++index;
    }

    //m_impl->panels.remove(t_sequence);

    emit gizmoContainerWillBeRemoved(t_gizmo, index);
    m_impl->gizmos.removeOne(t_gizmo);

    disconnect(t_gizmo, &SurfaceGizmoContainer::gizmoWasAdded,this, &Surface::surfaceWasModified);
    emit gizmoContainerWasRemoved(t_gizmo, index);
    emit surfaceWasModified();
}

void Surface::processChannels(ProcessContext &t_context, double lastTime)
{
    for(auto container : m_impl->gizmos)
    {
        container->processChannels(t_context, lastTime);
    }

    /*
    for(auto layer : m_impl->layers)
        layer->processChannels(t_context);
*/

}

void Surface::restore(Project &t_project)
{

}

void Surface::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json["uniqueId"].toString().toLatin1();

    if(t_json.contains("gizmos"))
    {
        QJsonArray gizmoArray = t_json.value("gizmos").toArray();
        for(const auto &gizmo : gizmoArray)
        {
            const QJsonObject &gizmoObj = gizmo.toObject();

            SurfaceGizmoContainer *c = new SurfaceGizmoContainer;
            c->readFromJson(gizmoObj, t_context);
            m_impl->gizmos.append(c);

            connect(c, &SurfaceGizmoContainer::gizmoWasAdded,this, &Surface::surfaceWasModified);
        }
    }
}

void Surface::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    QJsonArray gizmoArray;
    for(const auto &gizmo : m_impl->gizmos)
    {
        QJsonObject gizmoObj;
        gizmo->writeToJson(gizmoObj);
        gizmoArray.append(gizmoObj);
    }
    t_json.insert("gizmos", gizmoArray);
}


} // namespace photon

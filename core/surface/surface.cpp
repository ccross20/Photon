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
#include "containergizmo.h"
#include "surfacegraph.h"
#include "plugin/pluginfactory.h"

namespace photon {

Surface::Impl::Impl(Surface *t_facade):facade(t_facade)
{
    uniqueId = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);

}


Surface::Surface(const QString &t_name, QObject *parent)
    : QObject{parent},m_impl(new Impl(this))
{
    m_impl->name = t_name;
    m_impl->root = new ContainerGizmo;
    m_impl->root->setId("root");
    connect(m_impl->root, &ContainerGizmo::childrenChanged, this, &Surface::surfaceWasModified);
}

ContainerGizmo *Surface::rootContainer() const
{
    return m_impl->root;
}

Surface::~Surface()
{
    delete m_impl->root;
    delete m_impl;
}

void Surface::init()
{

}

QVector<SurfaceGizmo*> Surface::gizmos() const
{
    // All gizmos in the tree (leaves + containers). Containers have no outputs,
    // so including them is harmless for the value bus.
    QVector<SurfaceGizmo*> results;
    m_impl->root->collectDescendants(results, true);
    return results;
}

QVector<SurfaceGizmo*> Surface::gizmosByType(const QByteArray &t_type) const
{
    QVector<SurfaceGizmo*> results;
    for(auto *gizmo : gizmos())
    {
        if(gizmo->type() == t_type)
            results.append(gizmo);
    }
    return results;
}

SurfaceGizmo *Surface::findGizmoWithId(const QByteArray &t_id) const
{
    for(auto *gizmo : gizmos())
    {
        if(gizmo->id() == t_id)
            return gizmo;
    }
    qDebug() << "Could not find gizmo with id:" << t_id;
    return nullptr;
}

SurfaceGizmo *Surface::findGizmoWithUniqueId(const QByteArray &t_id) const
{
    if(auto *gizmo = m_impl->root->findDescendantWithUniqueId(t_id))
        return gizmo;

    qDebug() << "Could not find gizmo with id:" << t_id;
    return nullptr;
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

SurfaceGraph *Surface::graph() const
{
    return m_impl->graph;
}

void Surface::addGizmoContainer(photon::SurfaceGizmoContainer *t_gizmo)
{
    if(m_impl->gizmos.contains(t_gizmo))
        return;
    emit gizmoContainerWillBeAdded(t_gizmo, m_impl->gizmos.length());
    m_impl->gizmos.append(t_gizmo);

    connect(t_gizmo, &SurfaceGizmoContainer::gizmoWasAdded,this, &Surface::surfaceWasModified);
    connect(t_gizmo, &SurfaceGizmoContainer::gizmoWasRemoved,this, &Surface::surfaceWasModified);

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
    m_impl->root->restore(t_project);
}

void Surface::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json["uniqueId"].toString().toLatin1();

    if(t_json.contains("root"))
        m_impl->root->readFromJson(t_json.value("root").toObject(), t_context);
}

void Surface::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    QJsonObject rootObj;
    m_impl->root->writeToJson(rootObj);
    t_json.insert("root", rootObj);
}


} // namespace photon

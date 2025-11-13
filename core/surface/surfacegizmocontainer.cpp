#include "surfacegizmocontainer.h"
#include "viewer/surfacegizmocontainerwidget.h"
#include "model/node.h"
#include "model/parameter/parameter.h"
#include "fixtureaction.h"
#include "surfacegizmo.h"
#include "surfaceaction.h"
#include "canvasaction.h"
#include "gizmofactory.h"
#include "togglegizmo.h"

namespace photon {

class SurfaceGizmoContainer::Impl
{
public:
    Impl();
    ~Impl();

    QVector<SurfaceGizmo*> gizmos;
    QVector<SurfaceAction*> actions;
};

SurfaceGizmoContainer::Impl::Impl()
{


}

SurfaceGizmoContainer::Impl::~Impl()
{

}

SurfaceGizmoContainer::SurfaceGizmoContainer(QObject *parent):QObject(parent),m_impl(new Impl)
{
    m_impl->actions.append(new FixtureAction(this));
}

SurfaceGizmoContainer::~SurfaceGizmoContainer()
{
    delete m_impl;
}

void SurfaceGizmoContainer::processChannels(ProcessContext &t_context, double t_lastTime)
{
    //m_impl->elapsed = m_impl->timer.elapsed() / 1000.0;

    for(auto action : m_impl->actions)
    {
        if(!action->enableOnGizmoId().isEmpty())
        {
            auto enableOn = findGizmoWithUniqueId(action->enableOnGizmoId());

            auto enableOnToggle = dynamic_cast<ToggleGizmo*>(enableOn);
            if(enableOnToggle && !enableOnToggle->isPressed())
                continue;
        }
        action->processChannels(t_context);
    }
}

const QVector<SurfaceGizmo*> &SurfaceGizmoContainer::gizmos() const
{
    return m_impl->gizmos;
}

QVector<SurfaceGizmo*> SurfaceGizmoContainer::gizmosByType(const QByteArray &t_type) const
{
    QVector<SurfaceGizmo*> results;

    for(auto gizmo : m_impl->gizmos)
    {
        if(gizmo->type() == t_type)
        {
            results.append(gizmo);
        }
    }

    return results;
}

SurfaceGizmo *SurfaceGizmoContainer::findGizmoWithId(const QByteArray &t_id) const
{
    if(t_id.isEmpty())
        return nullptr;
    for(auto gizmo : m_impl->gizmos)
    {
        if(gizmo->id() == t_id)
        {
            return gizmo;
        }
    }
    return nullptr;
}

SurfaceGizmo *SurfaceGizmoContainer::findGizmoWithUniqueId(const QByteArray &t_id) const
{
    if(t_id.isEmpty())
        return nullptr;
    for(auto gizmo : m_impl->gizmos)
    {
        if(gizmo->uniqueId() == t_id)
        {
            return gizmo;
        }
    }
    return nullptr;
}

const QVector<SurfaceAction*> &SurfaceGizmoContainer::actions() const
{
    return m_impl->actions;
}

SurfaceGizmoContainerWidget *SurfaceGizmoContainer::createWidget(SurfaceMode mode)
{
    return new SurfaceGizmoContainerWidget(this, mode);
}

void SurfaceGizmoContainer::addGizmo(photon::SurfaceGizmo *t_gizmo)
{
    if(m_impl->gizmos.contains(t_gizmo))
        return;
    emit gizmoWillBeAdded(t_gizmo, m_impl->gizmos.length());
    m_impl->gizmos.append(t_gizmo);
    t_gizmo->setId("Gizmo "  + QByteArray::number(m_impl->gizmos.length()));
    t_gizmo->setParent(this);
    emit gizmoWasAdded(t_gizmo, m_impl->gizmos.length()-1);
}

void SurfaceGizmoContainer::removeGizmo(photon::SurfaceGizmo *t_gizmo)
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

    emit gizmoWillBeRemoved(t_gizmo, index);
    m_impl->gizmos.removeOne(t_gizmo);
    emit gizmoWasRemoved(t_gizmo, index);
}

void SurfaceGizmoContainer::addAction(photon::SurfaceAction *t_action)
{
    if(m_impl->actions.contains(t_action))
        return;
    emit actionWillBeAdded(t_action, m_impl->actions.length());
    m_impl->actions.append(t_action);
    t_action->setId("Action "  + QByteArray::number(m_impl->actions.length()));
    t_action->setParent(this);
    emit actionWasAdded(t_action, m_impl->actions.length()-1);
}

void SurfaceGizmoContainer::removeAction(photon::SurfaceAction *t_action)
{
    if(!m_impl->actions.contains(t_action))
        return;

    int index = 0;
    for(auto seq : m_impl->actions)
    {
        if(seq == t_action)
            break;
        ++index;
    }

    //m_impl->panels.remove(t_sequence);

    emit actionWillBeRemoved(t_action, index);
    m_impl->actions.removeOne(t_action);
    emit actionWasRemoved(t_action, index);
}

void SurfaceGizmoContainer::restore(Project &t_project)
{
    for(auto source : std::as_const(m_impl->gizmos))
    {
        source->restore(t_project);
    }

    QJsonArray actionArray;
    for(auto source : std::as_const(m_impl->actions))
    {
        source->restore(t_project);
    }
}

void SurfaceGizmoContainer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    if(t_json.contains("gizmos"))
    {
        m_impl->gizmos.clear();
        auto sourceArray = t_json.value("gizmos").toArray();
        for(const auto source : std::as_const(sourceArray))
        {
            auto sourceObj = source.toObject();
            auto newGizmo = GizmoFactory::createGizmo(sourceObj.value("type").toString().toLatin1());
            if(newGizmo)
            {
                newGizmo->readFromJson(sourceObj, t_context);

                m_impl->gizmos.append(newGizmo);
                newGizmo->setParent(this);
            }

        }
    }

    if(t_json.contains("actions"))
    {
        m_impl->actions.clear();
        auto sourceArray = t_json.value("actions").toArray();
        for(const auto source : std::as_const(sourceArray))
        {
            auto sourceObj = source.toObject();
            SurfaceAction *newAction = nullptr;
            if(sourceObj.value("type").toString() == "fixture")
                newAction = new FixtureAction(this);
            else if(sourceObj.value("type").toString() == "canvas")
                newAction = new CanvasAction(this);
            else
                newAction = new SurfaceAction(this);
            newAction->readFromJson(sourceObj, t_context);

            m_impl->actions.append(newAction);
        }
    }
}

void SurfaceGizmoContainer::writeToJson(QJsonObject &t_json) const
{
    QJsonArray gizmoArray;
    for(auto source : std::as_const(m_impl->gizmos))
    {
        QJsonObject sourceObj;
        source->writeToJson(sourceObj);
        gizmoArray.append(sourceObj);
    }
    t_json.insert("gizmos", gizmoArray);

    QJsonArray actionArray;
    for(auto source : std::as_const(m_impl->actions))
    {
        QJsonObject sourceObj;
        source->writeToJson(sourceObj);
        actionArray.append(sourceObj);
    }
    t_json.insert("actions", actionArray);

}

} // namespace photon

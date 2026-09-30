#include "propertycontroller.h"
#include "propertysubject.h"

namespace photon {

PropertyController::PropertyController() = default;
PropertyController::~PropertyController() = default;

PropertyController *PropertyController::instance()
{
    static PropertyController controller;
    return &controller;
}

void PropertyController::setSubject(PropertySubject *t_subject)
{
    // Re-selecting the same target would otherwise rebuild the page under the
    // user mid-edit; selection sources fire liberally (every scene
    // selectionChanged, every gizmo click) so this guard matters.
    if (t_subject && m_subject && t_subject->address() == m_subject->address()) {
        delete t_subject;
        return;
    }

    m_subject.reset(t_subject);
    emit subjectChanged(m_subject.get());
}

PropertySubject *PropertyController::subject() const
{
    return m_subject.get();
}

void PropertyController::selectNode(keira::Node *t_node)
{
    setSubject(PropertySubjectFactory::forNode(t_node));
}

void PropertyController::selectGizmo(Surface *t_surface, SurfaceGizmo *t_gizmo)
{
    setSubject(PropertySubjectFactory::forGizmo(t_surface, t_gizmo));
}

void PropertyController::selectResource(ProjectResource *t_resource)
{
    setSubject(PropertySubjectFactory::forResource(t_resource));
}

void PropertyController::selectChannelEffect(ChannelEffect *t_effect)
{
    setSubject(PropertySubjectFactory::forChannelEffect(t_effect));
}

void PropertyController::selectClip(Clip *t_clip)
{
    setSubject(PropertySubjectFactory::forClip(t_clip));
}

void PropertyController::clear()
{
    setSubject(nullptr);
}

} // namespace photon

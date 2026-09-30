#include <QWidget>
#include "propertysubject.h"
#include "gizmopropertyeditor.h"
#include "photoncore.h"
#include "project/project.h"
#include "project/projectresource.h"
#include "surface/surface.h"
#include "surface/surfacecollection.h"
#include "routine/routinecollection.h"
#include "routine/routine.h"
#include "fixture/fixturegroup.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"
#include "sequence/sequencecollection.h"
#include "sequence/sequence.h"
#include "graph/bus/busgraph.h"
#include "surface/surfacegizmo.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "model/graph.h"
#include "model/node.h"
#include "model/subgraphnode.h"
#include "view/nodeeditor.h"
#include "sequence/channel.h"
#include "sequence/channeleffect.h"
#include "sequence/layer.h"
#include "sequence/cliplayer.h"
#include "sequence/clip.h"
#include "sequence/viewer/clippropertyeditor.h"

namespace photon {

// ---------------------------------------------------------------------------
// Node

NodePropertySubject::NodePropertySubject(keira::Graph *t_graph, const QByteArray &t_nodeId)
    : m_graph(t_graph), m_nodeId(t_nodeId)
{
}

keira::Node *NodePropertySubject::node() const
{
    return m_graph ? m_graph->findNode(m_nodeId) : nullptr;
}

bool NodePropertySubject::isValid() const
{
    return node() != nullptr;
}

QString NodePropertySubject::title() const
{
    keira::Node *n = node();
    return n ? n->name() : QStringLiteral("(missing node)");
}

PropertyAddress NodePropertySubject::address() const
{
    PropertyAddress address;
    if (!m_graph)
        return address;

    // Root graph first, then each nested subgraph. Every graph below the root is
    // labelled by the node that contains it (its user-facing name) rather than
    // the graph's own fixed type name - the same choice GraphWidget's
    // breadcrumbs make, because "Subgraph" says nothing about which node it is.
    const QVector<keira::Graph *> chain = m_graph->ancestryChain();
    for (int i = 0; i < chain.size(); ++i) {
        keira::Graph *g = chain.at(i);
        const QString label = (i == 0 || !g->parentNode()) ? g->name() : g->parentNode()->name();
        address.append(PropertyAddress::KindGraph, g->uniqueId(), label);
    }

    address.append(PropertyAddress::KindNode, m_nodeId, title());
    return address;
}

QWidget *NodePropertySubject::createEditor()
{
    keira::Node *n = node();
    if (!n)
        return nullptr;

    // NodeEditor already builds a node's parameter page (including the
    // createCustomWidget() hook); it just used to live in GraphWidget's
    // splitter instead of here.
    keira::NodeEditor *editor = new keira::NodeEditor;
    editor->setNode(n);
    return editor;
}

// ---------------------------------------------------------------------------
// Gizmo

GizmoPropertySubject::GizmoPropertySubject(Surface *t_surface, const QByteArray &t_gizmoId)
    : m_surface(t_surface), m_gizmoId(t_gizmoId)
{
}

SurfaceGizmo *GizmoPropertySubject::gizmo() const
{
    return m_surface ? m_surface->findGizmoWithUniqueId(m_gizmoId) : nullptr;
}

bool GizmoPropertySubject::isValid() const
{
    return gizmo() != nullptr;
}

QString GizmoPropertySubject::title() const
{
    SurfaceGizmo *g = gizmo();
    if (!g)
        return QStringLiteral("(missing element)");
    // Gizmos carry a user-facing name in an identity property; fall back to the
    // type when it hasn't been set.
    const QVariant name = g->propertyValue("name");
    const QString text = name.toString();
    return text.isEmpty() ? g->gizmoTypeString() : text;
}

PropertyAddress GizmoPropertySubject::address() const
{
    PropertyAddress address;
    if (!m_surface)
        return address;
    address.append(PropertyAddress::KindSurface, m_surface->uniqueId(), m_surface->resourceName());
    address.append(PropertyAddress::KindGizmo, m_gizmoId, title());
    return address;
}

QWidget *GizmoPropertySubject::createEditor()
{
    SurfaceGizmo *g = gizmo();
    return g ? new GizmoPropertyEditor(g) : nullptr;
}

// ---------------------------------------------------------------------------
// Resource

ResourcePropertySubject::ResourcePropertySubject(ProjectResource *t_resource)
{
    if (t_resource) {
        m_resourceId = t_resource->resourceId();
        m_typeId = t_resource->resourceTypeId();
        m_guard = t_resource->resourceObject();
    }
}

ProjectResource *ResourcePropertySubject::resource() const
{
    if (!m_guard)
        return nullptr;
    // The guard proves the underlying QObject is alive; the resource mixin
    // shares its lifetime, so this cross-cast is safe while it holds.
    return dynamic_cast<ProjectResource *>(m_guard.data());
}

bool ResourcePropertySubject::isValid() const
{
    return resource() != nullptr;
}

QString ResourcePropertySubject::title() const
{
    ProjectResource *r = resource();
    return r ? r->resourceName() : QStringLiteral("(missing resource)");
}

PropertyAddress ResourcePropertySubject::address() const
{
    PropertyAddress address;
    ProjectResource *r = resource();
    if (!r)
        return address;

    // A scene object addresses as rig/<ancestors...>/object so the breadcrumbs
    // read the way the rig tree does; anything else is a flat project resource.
    if (auto *obj = dynamic_cast<SceneObject *>(r)) {
        address.append(PropertyAddress::KindRig, QByteArray(), QStringLiteral("Rig"));

        QVector<SceneObject *> chain;
        for (SceneObject *o = obj; o; o = o->parentSceneObject())
            chain.prepend(o);
        // The scene root is an implicit container, not something to show.
        for (int i = 0; i < chain.size(); ++i) {
            SceneObject *o = chain.at(i);
            if (!o->parentSceneObject())
                continue;
            address.append(PropertyAddress::KindObject, o->uniqueId(), o->resourceName());
        }
        return address;
    }

    address.append(PropertyAddress::KindProject, QByteArray(), QStringLiteral("Project"));
    address.append(PropertyAddress::KindResource, m_resourceId, title());
    return address;
}

QWidget *ResourcePropertySubject::createEditor()
{
    ProjectResource *r = resource();
    return r ? r->createResourceEditor() : nullptr;
}

// ---------------------------------------------------------------------------
// Channel effect

ChannelEffectPropertySubject::ChannelEffectPropertySubject(Channel *t_channel, const QByteArray &t_effectId)
    : m_channel(t_channel), m_effectId(t_effectId)
{
}

ChannelEffect *ChannelEffectPropertySubject::effect() const
{
    if (!m_channel)
        return nullptr;
    for (int i = 0; i < m_channel->effectCount(); ++i) {
        ChannelEffect *e = m_channel->effectAtIndex(i);
        if (e && e->uniqueId() == m_effectId)
            return e;
    }
    return nullptr;
}

bool ChannelEffectPropertySubject::isValid() const
{
    return effect() != nullptr;
}

QString ChannelEffectPropertySubject::title() const
{
    ChannelEffect *e = effect();
    return e ? e->name() : QStringLiteral("(missing effect)");
}

PropertyAddress ChannelEffectPropertySubject::address() const
{
    PropertyAddress address;
    ChannelEffect *e = effect();
    if (!e)
        return address;
    address.append(PropertyAddress::KindChannelEffect, e->uniqueId(), title());
    return address;
}

QWidget *ChannelEffectPropertySubject::createEditor()
{
    ChannelEffect *e = effect();
    return e ? e->createPropertyEditor() : nullptr;
}

// ---------------------------------------------------------------------------
// Clip

ClipPropertySubject::ClipPropertySubject(Clip *t_clip) : m_clip(t_clip)
{
}

Clip *ClipPropertySubject::clip() const
{
    return m_clip.data();
}

bool ClipPropertySubject::isValid() const
{
    // A clip removed from its layer is on its way to being deleted.
    return m_clip && m_clip->layer();
}

QString ClipPropertySubject::title() const
{
    if(!isValid())
        return QStringLiteral("(missing clip)");
    return m_clip->name().isEmpty() ? QStringLiteral("Clip") : m_clip->name();
}

PropertyAddress ClipPropertySubject::address() const
{
    PropertyAddress address;
    if(!isValid())
        return address;
    if(Sequence *sequence = m_clip->sequence())
        address.append(PropertyAddress::KindResource, sequence->resourceId(), sequence->resourceName());
    address.append(PropertyAddress::KindClip, m_clip->uniqueId(), title());
    return address;
}

QWidget *ClipPropertySubject::createEditor()
{
    return isValid() ? new ClipPropertyEditor(m_clip) : nullptr;
}

// ---------------------------------------------------------------------------
// Factory

namespace PropertySubjectFactory {

PropertySubject *forNode(keira::Node *t_node)
{
    if (!t_node || !t_node->graph())
        return nullptr;
    return new NodePropertySubject(t_node->graph(), t_node->uniqueId());
}

PropertySubject *forGizmo(Surface *t_surface, SurfaceGizmo *t_gizmo)
{
    if (!t_surface || !t_gizmo)
        return nullptr;
    return new GizmoPropertySubject(t_surface, t_gizmo->uniqueId());
}

PropertySubject *forResource(ProjectResource *t_resource)
{
    if (!t_resource)
        return nullptr;
    return new ResourcePropertySubject(t_resource);
}

PropertySubject *forClip(Clip *t_clip)
{
    if (!t_clip || !t_clip->layer())
        return nullptr;
    return new ClipPropertySubject(t_clip);
}

PropertySubject *forChannelEffect(ChannelEffect *t_effect)
{
    if (!t_effect || !t_effect->channel())
        return nullptr;
    return new ChannelEffectPropertySubject(t_effect->channel(), t_effect->uniqueId());
}

namespace {

// Depth-first search for a graph by id, starting from a root.
keira::Graph *findGraph(keira::Graph *root, const QByteArray &id)
{
    if (!root)
        return nullptr;
    if (root->uniqueId() == id)
        return root;
    for (keira::Node *node : root->nodes()) {
        // SubGraphNode::graph() is the INNER graph it wraps (it hides
        // Node::graph(), which is the outer one) - that inner graph is what a
        // node address can descend into.
        if (auto *subNode = dynamic_cast<keira::SubGraphNode *>(node)) {
            if (keira::Graph *found = findGraph(subNode->graph(), id))
                return found;
        }
    }
    return nullptr;
}

// Every graph root the project exposes: the bus, plus each routine.
keira::Graph *findGraphAnywhere(const QByteArray &id)
{
    Project *project = photonApp ? photonApp->project() : nullptr;
    if (!project)
        return nullptr;

    if (keira::Graph *found = findGraph(project->bus(), id))   // BusGraph is a keira::Graph
        return found;

    for (auto *routine : project->routines()->routines()) {
        if (keira::Graph *found = findGraph(routine, id))
            return found;
    }
    return nullptr;
}

// Depth-first through a channel's own sub-channels (Channel::subChannels()).
ChannelEffect *findEffectInChannel(Channel *channel, const QByteArray &effectId)
{
    if (!channel)
        return nullptr;
    for (int i = 0; i < channel->effectCount(); ++i) {
        ChannelEffect *e = channel->effectAtIndex(i);
        if (e && e->uniqueId() == effectId)
            return e;
    }
    for (Channel *sub : channel->subChannels()) {
        if (ChannelEffect *found = findEffectInChannel(sub, effectId))
            return found;
    }
    return nullptr;
}

// A layer is a ClipLayer - channels live on its clips - dispatch on whichever
// concrete type this one is.
ChannelEffect *findEffectInLayer(Layer *layer, const QByteArray &effectId)
{
    if (!layer)
        return nullptr;
    if (auto *clipLayer = dynamic_cast<ClipLayer *>(layer)) {
        for (Clip *clip : clipLayer->clips()) {
            for (Channel *channel : clip->channels()) {
                if (ChannelEffect *found = findEffectInChannel(channel, effectId))
                    return found;
            }
        }
    }
    return nullptr;
}

// Every channel effect reachable from every open sequence - needed to restore
// a pinned tab (which starts from a bare id, with no live pointer to start
// from) the same way findGraphAnywhere/findResourceAnywhere do for their kinds.
ChannelEffect *findEffectAnywhere(const QByteArray &effectId)
{
    if (!photonApp || !photonApp->sequences())
        return nullptr;
    for (Sequence *sequence : photonApp->sequences()->sequences()) {
        for (Layer *layer : sequence->layers()) {
            if (ChannelEffect *found = findEffectInLayer(layer, effectId))
                return found;
        }
    }
    return nullptr;
}

Clip *findClipAnywhere(const QByteArray &clipId)
{
    if (!photonApp || !photonApp->sequences())
        return nullptr;
    for (Sequence *sequence : photonApp->sequences()->sequences()) {
        for (Layer *layer : sequence->layers()) {
            if (auto *clipLayer = dynamic_cast<ClipLayer *>(layer)) {
                for (Clip *clip : clipLayer->clips())
                    if (clip->uniqueId() == clipId)
                        return clip;
            }
        }
    }
    return nullptr;
}

ProjectResource *findResourceAnywhere(const QByteArray &id)
{
    Project *project = photonApp ? photonApp->project() : nullptr;
    if (!project)
        return nullptr;

    for (auto *object : SceneIterator::ToList(project->sceneRoot()))
        if (object->resourceId() == id) return object;
    for (auto *group : project->groups()->groups())
        if (group->resourceId() == id) return group;
    for (auto *routine : project->routines()->routines())
        if (routine->resourceId() == id) return routine;
    for (auto *surface : project->surfaces()->surfaces())
        if (surface->resourceId() == id) return surface;
    for (auto *layout : project->pixelLayouts()->layouts())
        if (layout->resourceId() == id) return layout;
    if (photonApp && photonApp->sequences())
        for (auto *sequence : photonApp->sequences()->sequences())
            if (sequence->resourceId() == id) return sequence;

    return nullptr;
}

} // namespace

PropertySubject *resolve(const PropertyAddress &t_address)
{
    if (t_address.isEmpty())
        return nullptr;

    const PropertyAddress::Segment leaf = t_address.last();

    if (leaf.kind == PropertyAddress::KindNode) {
        // The segment before the node is its owning graph.
        if (t_address.size() < 2)
            return nullptr;
        const PropertyAddress::Segment graphSeg = t_address.segments().at(t_address.size() - 2);
        keira::Graph *graph = findGraphAnywhere(graphSeg.id);
        if (!graph || !graph->findNode(leaf.id))
            return nullptr;
        return new NodePropertySubject(graph, leaf.id);
    }

    if (leaf.kind == PropertyAddress::KindGizmo) {
        if (t_address.size() < 2)
            return nullptr;
        const PropertyAddress::Segment surfaceSeg = t_address.segments().at(t_address.size() - 2);
        Project *project = photonApp ? photonApp->project() : nullptr;
        if (!project)
            return nullptr;
        Surface *surface = project->surfaces()->findSurfaceWithId(surfaceSeg.id);
        if (!surface || !surface->findGizmoWithUniqueId(leaf.id))
            return nullptr;
        return new GizmoPropertySubject(surface, leaf.id);
    }

    if (leaf.kind == PropertyAddress::KindObject || leaf.kind == PropertyAddress::KindResource
        || leaf.kind == PropertyAddress::KindSurface) {
        ProjectResource *resource = findResourceAnywhere(leaf.id);
        return resource ? new ResourcePropertySubject(resource) : nullptr;
    }

    if (leaf.kind == PropertyAddress::KindGraph) {
        keira::Graph *graph = findGraphAnywhere(leaf.id);
        if (!graph)
            return nullptr;

        // A routine is both a graph and a resource (see routine.h) - it has
        // its own editor, so show that directly.
        if (auto *resource = dynamic_cast<ProjectResource *>(graph))
            return new ResourcePropertySubject(resource);

        // A subgraph has no properties of its own; what it means to select it
        // is to select the node that owns it - that node's parameter page
        // (including any custom widget) is what represents this subgraph
        // everywhere else in the app.
        if (keira::Node *owner = graph->parentNode())
            return new NodePropertySubject(owner->graph(), owner->uniqueId());

        // The root graph (the Bus) is neither of those - nothing to show.
        return nullptr;
    }

    if (leaf.kind == PropertyAddress::KindClip) {
        Clip *clip = findClipAnywhere(leaf.id);
        return clip ? new ClipPropertySubject(clip) : nullptr;
    }

    if (leaf.kind == PropertyAddress::KindChannelEffect) {
        ChannelEffect *effect = findEffectAnywhere(leaf.id);
        if (!effect || !effect->channel())
            return nullptr;
        return new ChannelEffectPropertySubject(effect->channel(), leaf.id);
    }

    return nullptr;
}

} // namespace PropertySubjectFactory

} // namespace photon

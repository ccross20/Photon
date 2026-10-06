#include <QMimeData>
#include "savedresourcedrop.h"
#include "savedcolornode.h"
#include "savedgradientnode.h"
#include "savedpalettenode.h"
#include "graph/node/fixture/selectfixturesnode.h"
#include "project/projectmodel.h"
#include "project/projectresource.h"
#include "color/colorresource.h"
#include "color/gradientresource.h"
#include "color/colorpaletteresource.h"
#include "tag/tagmime.h"
#include "scene/scenefalloff.h"
#include "scene/scenemodel.h"

namespace photon {

QVector<keira::ExternalDropNodeSpec> projectResourceDropInterpreter(const QMimeData *t_mimeData)
{
    QVector<keira::ExternalDropNodeSpec> specs;

    for(ProjectResource *resource : decodeProjectResourceMime(t_mimeData))
    {
        if(!resource)
            continue;

        if(dynamic_cast<ColorResource*>(resource))
        {
            specs.append({SavedColorNode::info().nodeId, SavedColorNode::ColorParam, QString::fromUtf8(resource->resourceId())});
        }
        else if(dynamic_cast<GradientResource*>(resource))
        {
            specs.append({SavedGradientNode::info().nodeId, SavedGradientNode::GradientParam, QString::fromUtf8(resource->resourceId())});
        }
        else if(dynamic_cast<ColorPaletteResource*>(resource))
        {
            specs.append({SavedPaletteNode::info().nodeId, SavedPaletteNode::PaletteParam, QString::fromUtf8(resource->resourceId())});
        }
    }

    // A Falloff dragged from the project panel becomes a Spatial Falloff node
    // pointed at it. That node lives in the nodes plugin, so it's named by id.
    for(SceneObject *object : decodeSceneObjectMime(t_mimeData))
    {
        if(auto *falloff = dynamic_cast<SceneFalloff*>(object))
            specs.append({QByteArrayLiteral("photon.falloff.spatial"), QByteArrayLiteral("helper"),
                          QString::fromUtf8(falloff->uniqueId())});
    }

    if(QStringList tags = decodeTagMime(t_mimeData); !tags.isEmpty())
    {
        specs.append({SelectFixturesNode::info().nodeId, SelectFixturesNode::TagsParam, tags.join(' ')});
    }

    return specs;
}

} // namespace photon

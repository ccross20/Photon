#include <QMimeData>
#include "savedresourcedrop.h"
#include "savedcolornode.h"
#include "savedgradientnode.h"
#include "savedpalettenode.h"
#include "project/projectmodel.h"
#include "project/projectresource.h"
#include "color/colorresource.h"
#include "color/gradientresource.h"
#include "color/colorpaletteresource.h"

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

    return specs;
}

} // namespace photon

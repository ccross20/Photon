#include "savedpalettenode.h"
#include "graph/parameter/colorpaletteparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "color/colorpalettecollection.h"
#include "color/colorpaletteresource.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray SavedPaletteNode::PaletteParam = "palette";
const QByteArray SavedPaletteNode::ResultParam = "result";

class SavedPaletteNode::Impl
{
public:
    keira::StringOptionParameter *paletteParam = nullptr;
    ColorPaletteParameter *resultParam = nullptr;
};

keira::NodeInformation SavedPaletteNode::info()
{
    keira::NodeInformation toReturn([](){return new SavedPaletteNode;});
    toReturn.name = "Saved Color Palette";
    toReturn.nodeId = "photon.library.saved-palette";
    toReturn.categories = {"Color"};

    return toReturn;
}

SavedPaletteNode::SavedPaletteNode() : keira::Node("photon.library.saved-palette"), m_impl(new Impl)
{
    setName("Saved Color Palette");
}

SavedPaletteNode::~SavedPaletteNode()
{
    delete m_impl;
}

void SavedPaletteNode::createParameters()
{
    // Dropdown of the project's saved palettes (managed in the project
    // panel's Color Palettes section). Re-listed live each time the editor
    // builds the combo, same as FixtureGroupNode's group picker - but
    // stored/matched by uniqueId, not name, so renaming a saved palette
    // doesn't desync every node that references it.
    m_impl->paletteParam = new keira::StringOptionParameter(PaletteParam, "Palette", {}, 0);
    m_impl->paletteParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        if(Project *project = photonApp->project())
        {
            for(auto *palette : project->colorPalettes()->palettes())
                options.append({palette->name(), QString::fromUtf8(palette->uniqueId())});
        }
        return options;
    });
    addParameter(m_impl->paletteParam);

    m_impl->resultParam = new ColorPaletteParameter(ResultParam, "Palette", {}, keira::AllowMultipleOutput);
    addParameter(m_impl->resultParam);
}

void SavedPaletteNode::evaluate(keira::EvaluationContext *) const
{
    ColorPalette result;

    const QByteArray id = m_impl->paletteParam->value().toString().toUtf8();
    if(ColorPaletteResource *palette = photonApp->project()->colorPalettes()->findPaletteWithId(id))
        result = palette->palette();

    m_impl->resultParam->setValue(QVariant::fromValue(result));
}

} // namespace photon

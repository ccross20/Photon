#include "savedcolornode.h"
#include "graph/parameter/colorparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "color/colorcollection.h"
#include "color/colorresource.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray SavedColorNode::ColorParam = "color";
const QByteArray SavedColorNode::ResultParam = "result";

class SavedColorNode::Impl
{
public:
    keira::StringOptionParameter *colorParam = nullptr;
    ColorParameter *resultParam = nullptr;
};

keira::NodeInformation SavedColorNode::info()
{
    keira::NodeInformation toReturn([](){return new SavedColorNode;});
    toReturn.name = "Saved Color";
    toReturn.nodeId = "photon.library.saved-color";
    toReturn.categories = {"Color"};

    return toReturn;
}

SavedColorNode::SavedColorNode() : keira::Node("photon.library.saved-color"), m_impl(new Impl)
{
    setName("Saved Color");
}

SavedColorNode::~SavedColorNode()
{
    delete m_impl;
}

void SavedColorNode::createParameters()
{
    // Dropdown of the project's saved colours (managed in the project panel's
    // Colors section). Re-listed live each time the editor builds the combo,
    // same as FixtureGroupNode's group picker - but stored/matched by
    // uniqueId, not name, so renaming a saved colour doesn't desync every
    // node that references it.
    m_impl->colorParam = new keira::StringOptionParameter(ColorParam, "Color", {}, 0);
    m_impl->colorParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        if(Project *project = photonApp->project())
        {
            for(auto *color : project->colors()->colors())
                options.append({color->name(), QString::fromUtf8(color->uniqueId())});
        }
        return options;
    });
    addParameter(m_impl->colorParam);

    m_impl->resultParam = new ColorParameter(ResultParam, "Color", Qt::white, keira::AllowMultipleOutput);
    addParameter(m_impl->resultParam);
}

void SavedColorNode::evaluate(keira::EvaluationContext *) const
{
    QColor result = Qt::white;

    const QByteArray id = m_impl->colorParam->value().toString().toUtf8();
    if(ColorResource *color = photonApp->project()->colors()->findColorWithId(id))
        result = color->color();

    m_impl->resultParam->setValue(QVariant::fromValue(result));
}

} // namespace photon

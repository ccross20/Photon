#include "savedgradientnode.h"
#include "graph/parameter/gradientparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "color/gradientcollection.h"
#include "color/gradientresource.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray SavedGradientNode::GradientParam = "gradient";
const QByteArray SavedGradientNode::ResultParam = "result";

class SavedGradientNode::Impl
{
public:
    keira::StringOptionParameter *gradientParam = nullptr;
    GradientParameter *resultParam = nullptr;
};

keira::NodeInformation SavedGradientNode::info()
{
    keira::NodeInformation toReturn([](){return new SavedGradientNode;});
    toReturn.name = "Saved Gradient";
    toReturn.nodeId = "photon.library.saved-gradient";
    toReturn.categories = {"Gradient"};

    return toReturn;
}

SavedGradientNode::SavedGradientNode() : keira::Node("photon.library.saved-gradient"), m_impl(new Impl)
{
    setName("Saved Gradient");
}

SavedGradientNode::~SavedGradientNode()
{
    delete m_impl;
}

void SavedGradientNode::createParameters()
{
    // Dropdown of the project's saved gradients (managed in the project
    // panel's Gradients section). Re-listed live each time the editor builds
    // the combo, same as FixtureGroupNode's group picker - but stored/matched
    // by uniqueId, not name, so renaming a saved gradient doesn't desync every
    // node that references it.
    m_impl->gradientParam = new keira::StringOptionParameter(GradientParam, "Gradient", {}, 0);
    m_impl->gradientParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        if(Project *project = photonApp->project())
        {
            for(auto *gradient : project->gradients()->gradients())
                options.append({gradient->name(), QString::fromUtf8(gradient->uniqueId())});
        }
        return options;
    });
    addParameter(m_impl->gradientParam);

    m_impl->resultParam = new GradientParameter(ResultParam, "Gradient", Gradient{}, keira::AllowMultipleOutput);
    addParameter(m_impl->resultParam);
}

void SavedGradientNode::evaluate(keira::EvaluationContext *) const
{
    Gradient result;

    const QByteArray id = m_impl->gradientParam->value().toString().toUtf8();
    if(GradientResource *gradient = photonApp->project()->gradients()->findGradientWithId(id))
        result = gradient->gradient();

    m_impl->resultParam->setValue(QVariant::fromValue(result));
}

} // namespace photon

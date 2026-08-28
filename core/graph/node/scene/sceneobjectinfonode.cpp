#include "sceneobjectinfonode.h"
#include "model/parameter/stringoptionparameter.h"
#include "graph/parameter/matrixparameter.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "scene/scenemanager.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray SceneObjectInfoNode::ObjectParam = "object";
const QByteArray SceneObjectInfoNode::MatrixOutput = "matrixOutput";

class SceneObjectInfoNode::Impl
{
public:
    keira::StringOptionParameter *objectParam;
    MatrixParameter *matrixParam;
};

keira::NodeInformation SceneObjectInfoNode::info()
{
    keira::NodeInformation toReturn([](){return new SceneObjectInfoNode;});
    toReturn.name = "Scene Object Info";
    toReturn.nodeId = "photon.graph.scene-object-info";
    toReturn.categories = {"Scene"};

    return toReturn;
}

SceneObjectInfoNode::SceneObjectInfoNode() : keira::Node("photon.graph.scene-object-info"),m_impl(new Impl)
{
    setName("Scene Object Info");
}

SceneObjectInfoNode::~SceneObjectInfoNode()
{
    delete m_impl;
}

void SceneObjectInfoNode::createParameters()
{
    m_impl->objectParam = new keira::StringOptionParameter(ObjectParam, "Object", {}, 0);
    m_impl->objectParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        options.append(std::pair<QString, QString>("(none)", QString()));
        if(Project *project = photonApp->project())
        {
            for(SceneObject *object : SceneIterator::ToList(project->sceneRoot()))
            {
                if(object == project->sceneRoot())
                    continue;
                // Keyed by uniqueId (not name) so renaming an object doesn't
                // break an existing pick - the type suffix just helps tell
                // apart same-named objects in the dropdown.
                options.append(std::pair<QString, QString>(
                    object->name() + " (" + QString::fromUtf8(object->typeId()) + ")",
                    QString::fromUtf8(object->uniqueId())));
            }
        }
        return options;
    });
    addParameter(m_impl->objectParam);

    m_impl->matrixParam = new MatrixParameter(MatrixOutput, "Matrix", QMatrix4x4{}, keira::AllowMultipleOutput);
    addParameter(m_impl->matrixParam);
}

void SceneObjectInfoNode::evaluate(keira::EvaluationContext *) const
{
    const QByteArray objectId = m_impl->objectParam->value().toString().toUtf8();
    if(objectId.isEmpty() || !photonApp->project())
        return;

    SceneObject *object = photonApp->project()->scene()->findObjectById(objectId);
    if(!object)
        return;

    m_impl->matrixParam->setValue(QVariant::fromValue(object->globalMatrix()));
}

} // namespace photon

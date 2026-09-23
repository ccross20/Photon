#include "fixturegroupnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "fixture/fixturegroup.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray FixtureGroupNode::GroupParam = "group";
const QByteArray FixtureGroupNode::ResultParam = "result";

class FixtureGroupNode::Impl
{
public:
    keira::StringOptionParameter *groupParam = nullptr;
    FixtureListParameter *resultParam = nullptr;
};

keira::NodeInformation FixtureGroupNode::info()
{
    keira::NodeInformation toReturn([](){return new FixtureGroupNode;});
    toReturn.name = "Fixture Group";
    toReturn.nodeId = "photon.fixture.group";
    toReturn.categories = {"Fixture List"};

    return toReturn;
}

FixtureGroupNode::FixtureGroupNode() : keira::Node("photon.fixture.group"), m_impl(new Impl)
{
    setName("Fixture Group");
}

FixtureGroupNode::~FixtureGroupNode()
{
    delete m_impl;
}

void FixtureGroupNode::createParameters()
{
    // Dropdown of the project's fixture groups (managed in the Fixture Groups panel).
    // The lambda re-lists groups whenever the editor builds the combo, so it stays in
    // sync with the panel. The stored value is the group's uniqueId, not its name, so
    // renaming a group in the panel doesn't desync a node that already points at it.
    m_impl->groupParam = new keira::StringOptionParameter(GroupParam, "Group", {}, 0);
    m_impl->groupParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        if(Project *project = photonApp->project())
        {
            for(FixtureGroup *group : project->groups()->groups())
                options.append({group->name(), QString::fromUtf8(group->uniqueId())});
        }
        return options;
    });
    addParameter(m_impl->groupParam);

    m_impl->resultParam = new FixtureListParameter(ResultParam, "Fixtures", {});
    addParameter(m_impl->resultParam);
}

void FixtureGroupNode::evaluate(keira::EvaluationContext *) const
{
    QVector<FixtureParameterData> result;

    const QString stored = m_impl->groupParam->value().toString();
    Project *project = photonApp->project();
    // Id lookup first; falls back to treating the stored value as a name for
    // projects saved before this node switched from name- to id-keyed storage.
    FixtureGroup *group = project->groups()->findGroupWithId(stored.toUtf8());
    if(!group)
        group = project->groups()->findGroup(stored);

    if(group)
    {
        for(Fixture *fixture : group->query().resolve(project))
            result.append(fixture);
    }

    m_impl->resultParam->setValue(QVariant::fromValue(result));
}

} // namespace photon

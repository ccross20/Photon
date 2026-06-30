#include "selectfixturesnode.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/stringparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "fixture/fixturequery.h"
#include "fixture/fixture.h"
#include "scene/scenezone.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray SelectFixturesNode::TagsParam = "tags";
const QByteArray SelectFixturesNode::TypeParam = "type";
const QByteArray SelectFixturesNode::ZoneParam = "zone";
const QByteArray SelectFixturesNode::ResultParam = "result";

class SelectFixturesNode::Impl
{
public:
    keira::StringParameter *tagsParam = nullptr;
    keira::StringParameter *typeParam = nullptr;
    keira::StringOptionParameter *zoneParam = nullptr;
    FixtureListParameter *resultParam = nullptr;
};

keira::NodeInformation SelectFixturesNode::info()
{
    keira::NodeInformation toReturn([](){return new SelectFixturesNode;});
    toReturn.name = "Select Fixtures";
    toReturn.nodeId = "photon.fixture.select";
    toReturn.categories = {"Fixture List"};

    return toReturn;
}

SelectFixturesNode::SelectFixturesNode() : keira::Node("photon.fixture.select"), m_impl(new Impl)
{
    setName("Select Fixtures");
}

SelectFixturesNode::~SelectFixturesNode()
{
    delete m_impl;
}

void SelectFixturesNode::createParameters()
{
    // Space-separated tags; a fixture must carry ALL of them (AND). Empty = any.
    m_impl->tagsParam = new keira::StringParameter(TagsParam, "Tags", "", 0);
    addParameter(m_impl->tagsParam);

    // Matched (case-insensitive substring) against the fixture's model type and its
    // OpenFixture categories — e.g. "mover", "moving", "par". Empty = any.
    m_impl->typeParam = new keira::StringParameter(TypeParam, "Type", "", 0);
    addParameter(m_impl->typeParam);

    // Optional spatial filter: a SceneZone the fixture must sit inside. The lambda
    // lists the project's zones (blank entry = no zone filter).
    m_impl->zoneParam = new keira::StringOptionParameter(ZoneParam, "Zone", {}, 0);
    m_impl->zoneParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        options.append(std::pair<QString, QString>("(none)", QString()));
        if(Project *project = photonApp->project())
        {
            for(const QString &name : SceneZone::zoneNames(project))
                options.append(std::pair<QString, QString>(name, name));
        }
        return options;
    });
    addParameter(m_impl->zoneParam);

    m_impl->resultParam = new FixtureListParameter(ResultParam, "Fixtures", {});
    addParameter(m_impl->resultParam);
}

void SelectFixturesNode::evaluate(keira::EvaluationContext *) const
{
    FixtureQuery query;
    query.tags = m_impl->tagsParam->value().toString().split(' ', Qt::SkipEmptyParts);
    query.type = m_impl->typeParam->value().toString().trimmed();
    query.zone = m_impl->zoneParam->value().toString().trimmed();

    QVector<FixtureParameterData> result;
    for(Fixture *fixture : query.resolve(photonApp->project()))
        result.append(fixture);

    m_impl->resultParam->setValue(QVariant::fromValue(result));
}

} // namespace photon

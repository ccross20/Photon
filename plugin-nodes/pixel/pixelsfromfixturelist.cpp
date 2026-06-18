#include "pixelsfromfixturelist.h"
#include "graph/parameter/fixturelistparameter.h"
#include "graph/parameter/pixellistparameter.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"

namespace photon {


keira::NodeInformation PixelsFromFixtureList::info()
{
    keira::NodeInformation toReturn([](){return new PixelsFromFixtureList;});
    toReturn.name = "Pixels from Fixtures";
    toReturn.nodeId = "photon.pixels.from-fixtures";
    toReturn.categories = {"Pixel"};

    return toReturn;
}


PixelsFromFixtureList::PixelsFromFixtureList() : keira::Node("photon.pixels.from-fixtures")
{
    setName("Pixels from Fixtures");
}

void PixelsFromFixtureList::createParameters()
{

    m_inParam = new FixtureListParameter("fixturesIn","Fixtures In",{});
    m_pixelsOut = new PixelListParameter("pixelsOut","Pixels",{});

    addParameter(m_inParam);
    addParameter(m_pixelsOut);
}

void PixelsFromFixtureList::evaluate(keira::EvaluationContext *t_context) const
{

    auto fixtures = m_inParam->value().value<QVector<FixtureParameterData>>();

    QVector<PixelParameterData> results;

    for(int i = 0; i < fixtures.length(); ++i)
    {
        auto fix = photonApp->project()->fixtures()->fixtureWithId(fixtures[i].fixtureId);

        for(int k = 0; k < fix->colorCount(); ++k)
            results.append(PixelParameterData(fix->uniqueId(), k));
    }

    m_pixelsOut->setValue(QVariant::fromValue(results));

}


} // namespace photon

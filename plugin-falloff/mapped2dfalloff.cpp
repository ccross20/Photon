#include <QVBoxLayout>
#include "mapped2dfalloff.h"
#include "fixture/fixture.h"

namespace photon {


FalloffEffectInformation Mapped2DFalloff::info()
{
    FalloffEffectInformation toReturn([](){return new Mapped2DFalloff;});
    toReturn.name = "Mapped 2D";
    toReturn.effectId = "photon.falloff.mapped2d";
    toReturn.categories.append("Modifier");

    return toReturn;
}


Mapped2DFalloffEditor::Mapped2DFalloffEditor(Mapped2DFalloff *t_effect):QWidget(),m_effect(t_effect)
{
    m_editor = new FixtureFalloff2DEditor;
    m_editor->setMap(m_effect->falloff());

    connect(m_editor, &FixtureFalloff2DEditor::mapUpdated, this, &Mapped2DFalloffEditor::mapUpdated);


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_editor);
    vLayout->addStretch();
    setLayout(vLayout);
}

void Mapped2DFalloffEditor::mapUpdated()
{
    m_effect->setFalloff(m_editor->map());
}


Mapped2DFalloff::Mapped2DFalloff()
{

}

double Mapped2DFalloff::falloff(Fixture *t_fixture) const
{
    if(previousEffect())
    {
        return previousEffect()->falloff(t_fixture) * m_map.falloff(t_fixture);
    }
    return m_map.falloff(t_fixture);
}

QWidget *Mapped2DFalloff::createEditor()
{
    return new Mapped2DFalloffEditor(this);
}

void Mapped2DFalloff::setFalloff(const FalloffMap2D &map)
{
    //qDebug() << "Updated";
    m_map = map;
    updated();
}

FalloffMap2D Mapped2DFalloff::falloff() const
{
    return m_map;
}


void Mapped2DFalloff::readFromJson(const QJsonObject &t_json)
{
    FalloffEffect::readFromJson(t_json);

    if(t_json.contains("map"))
    {
        m_map.readFromJson(t_json.value("map").toObject());
    }
}

void Mapped2DFalloff::writeToJson(QJsonObject &t_json) const
{
    FalloffEffect::writeToJson(t_json);

    QJsonObject mapObj;
    m_map.writeToJson(mapObj);
    t_json.insert("map", mapObj);
}

} // namespace photon

#include <QLabel>
#include "fixturelistparameter.h"
#include "model/parameter/parameter.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "model/node.h"

namespace photon {

const QByteArray FixtureListParameter::ParameterId = "fixtureList";

FixtureListParameter::FixtureListParameter(): Parameter() {}

FixtureListParameter::FixtureListParameter(const QByteArray &t_id, const QString &t_name, QVector<FixtureParameterData> t_default, int connectionOptions):
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions)
{

}

FixtureListParameter::~FixtureListParameter()
{
}

// A per-node checkbox tree of every fixture in the project doesn't scale and
// duplicates SelectFixturesNode's own (tag/type/zone-filtered) picker - this
// is a summary display only; wire a SelectFixturesNode upstream to choose a
// specific subset.
QWidget *FixtureListParameter::createWidget(keira::NodeEditor *) const
{
    auto *label = new QLabel();
    label->setMaximumHeight(30);
    label->setStyleSheet("background:transparent;");
    label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    updateWidget(label);
    return label;
}

void FixtureListParameter::updateWidget(QWidget *t_widget) const
{
    int count = resolvedValue().size();
    static_cast<QLabel*>(t_widget)->setText(count == 1 ? "1 fixture" : QString("%1 fixtures").arg(count));
}

QVariant FixtureListParameter::updateValue(QWidget *) const
{
    // Display-only widget now - not part of any commit path.
    return value();
}

QVector<FixtureParameterData> FixtureListParameter::resolvedValue() const
{
    auto stored = value().value<QVector<FixtureParameterData>>();
    if(!stored.isEmpty() || hasInput() || !photonApp->project())
        return stored;

    QVector<FixtureParameterData> all;
    for(auto *fixture : photonApp->project()->fixtures()->fixtures())
        all.append(fixture);
    return all;
}


void FixtureListParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    QJsonArray fixArray = t_json.value("value").toArray();

    QVector<FixtureParameterData> dataList;

    for(auto fix : fixArray)
    {
        auto fixObj = fix.toObject();

        FixtureParameterData data;
        data.fixtureId = fixObj.value("id").toString().toLatin1();
        data.offset = fixObj.value("offset").toDouble();

        dataList.append(data);
    }

    setValue(QVariant::fromValue(dataList));
}

void FixtureListParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    QJsonArray fixArray;

    QVector<FixtureParameterData> dataList = value().value<QVector<FixtureParameterData>>();

    for(auto data : dataList)
    {
        QJsonObject obj;
        obj.insert("id", QString(data.fixtureId));
        obj.insert("offset", data.offset);
        fixArray.append(obj);
    }

    t_json.insert("value",fixArray);
}

} // namespace photon

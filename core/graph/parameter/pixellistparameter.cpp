#include "pixellistparameter.h"

namespace photon {

const QByteArray PixelListParameter::ParameterId = "pixelList";

PixelListParameter::PixelListParameter() : Parameter(){}

PixelListParameter::PixelListParameter(const QByteArray &t_id, const QString &t_name, QVector<PixelParameterData> t_default, int connectionOptions):
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions)
{

}

void PixelListParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    QJsonArray fixArray = t_json.value("value").toArray();

    QVector<PixelParameterData> dataList;

    for(auto fix : fixArray)
    {
        auto fixObj = fix.toObject();

        PixelParameterData data;
        data.fixtureId = fixObj.value("id").toString().toLatin1();
        data.index = fixObj.value("index").toInt();

        dataList.append(data);
    }

    setValue(QVariant::fromValue(dataList));
}

void PixelListParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    QJsonArray fixArray;

    QVector<PixelParameterData> dataList = value().value<QVector<PixelParameterData>>();

    for(auto data : dataList)
    {
        QJsonObject obj;
        obj.insert("id", QString(data.fixtureId));
        obj.insert("index", data.index);
        fixArray.append(obj);
    }

    t_json.insert("value",fixArray);

}

} // namespace photon

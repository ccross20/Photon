#ifndef PIXELLISTPARAMETER_H
#define PIXELLISTPARAMETER_H
#include "photon-global.h"
#include "model/parameter/parameter.h"
#include "gui/sceneobjectselector.h"
#include "fixture/fixture.h"

namespace photon {


struct PixelParameterData
{
    PixelParameterData(){}
    PixelParameterData(QByteArray id, int index):fixtureId(id),index(index){}
    PixelParameterData(Fixture *fixture, int index):fixtureId(fixture->uniqueId()),index(index){}


    QByteArray fixtureId;
    int index = 0;

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    friend QDebug operator<< (QDebug debug, const PixelParameterData &t_data)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << "PixelParameterData " << t_data.fixtureId << t_data.index;

        return debug;
    }

    friend QDataStream & operator<< (QDataStream& stream, const PixelParameterData &t_data)
    {
        stream << static_cast<uchar>(1); // version number
        stream << t_data.fixtureId;
        stream << t_data.index;
        return stream;
    }
    friend QDataStream & operator>> (QDataStream& stream, PixelParameterData &t_data)
    {
        uchar version = 0;

        QByteArray id;
        int index;
        stream >> version;
        stream >> id;
        stream >> index;

        t_data.fixtureId = id;
        t_data.index = index;


        return stream;
    }
};

class PHOTONCORE_EXPORT PixelListParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    PixelListParameter();
    PixelListParameter(const QByteArray &t_id, const QString &t_name, QVector<PixelParameterData> t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);


    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:

};

} // namespace photon

Q_DECLARE_METATYPE(photon::PixelParameterData)

#endif // PIXELLISTPARAMETER_H

#ifndef FIXTURELISTPARAMETER_H
#define FIXTURELISTPARAMETER_H
#include "photon-global.h"
#include "model/parameter/parameter.h"
#include "fixture/fixture.h"

namespace photon {

struct FixtureParameterData
{
    FixtureParameterData(){}
    FixtureParameterData(QByteArray id):fixtureId(id){}
    FixtureParameterData(Fixture *fixture):fixtureId(fixture->uniqueId()){}


    QByteArray fixtureId;
    double offset = 0;

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    friend QDebug operator<< (QDebug debug, const FixtureParameterData &t_data)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << "FixtureParameterData " << t_data.fixtureId;

        return debug;
    }

    friend QDataStream & operator<< (QDataStream& stream, const FixtureParameterData &t_data)
    {
        stream << static_cast<uchar>(1); // version number
        stream << t_data.fixtureId;
        stream << t_data.offset;
        return stream;
    }
    friend QDataStream & operator>> (QDataStream& stream, FixtureParameterData &t_data)
    {
        uchar version = 0;

        QByteArray id;
        double offset;
        stream >> version;
        stream >> id;
        stream >> offset;

        t_data.fixtureId = id;
        t_data.offset = offset;


        return stream;
    }
};

class PHOTONCORE_EXPORT FixtureListParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    FixtureListParameter();
    FixtureListParameter(const QByteArray &t_id, const QString &t_name, QVector<FixtureParameterData> t_default, int connectionOptions = keira::AllowSingleInput);
    ~FixtureListParameter();

    QWidget *createWidget(keira::NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    // The value to actually use: value() as stored, unless this parameter is
    // unconnected and has nothing of its own set - then every fixture in the
    // project, so a fresh/default fixture list means "everything" rather
    // than "nothing" until a specific selection is wired in upstream (e.g.
    // a SelectFixturesNode).
    QVector<FixtureParameterData> resolvedValue() const;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
};

} // namespace photon


Q_DECLARE_METATYPE(photon::FixtureParameterData)

#endif // FIXTURELISTPARAMETER_H

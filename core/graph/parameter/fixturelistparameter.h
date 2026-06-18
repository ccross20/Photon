#ifndef FIXTURELISTPARAMETER_H
#define FIXTURELISTPARAMETER_H
#include "photon-global.h"
#include "model/parameter/parameter.h"
#include "gui/sceneobjectselector.h"
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

class FixtureListParameter;

class SelectorWatcher : public QObject
{
public:
    SelectorWatcher(FixtureListParameter *, SceneObjectSelector *);

public slots:
    void selectionChanged();

private:
    FixtureListParameter *m_param;
    SceneObjectSelector *m_selector;

};

class PHOTONCORE_EXPORT FixtureListParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    FixtureListParameter();
    FixtureListParameter(const QByteArray &t_id, const QString &t_name, QVector<FixtureParameterData> t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
    ~FixtureListParameter();

    QWidget *createWidget(keira::NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:

    class Impl;
    Impl *m_impl;

};

} // namespace photon


Q_DECLARE_METATYPE(photon::FixtureParameterData)

#endif // FIXTURELISTPARAMETER_H

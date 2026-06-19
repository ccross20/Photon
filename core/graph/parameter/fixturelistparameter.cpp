#include <QLabel>
#include "fixturelistparameter.h"
#include "model/parameter/parameter.h"
#include "photoncore.h"
#include "project/project.h"
#include "scene/sceneiterator.h"
#include "fixture/fixture.h"
#include "model/node.h"
#include "scene/scenemanager.h"

namespace photon {

const QByteArray FixtureListParameter::ParameterId = "fixtureList";

class FixtureListParameter::Impl
{
public:
    QVector<QByteArray> selected;
};

FixtureListParameter::FixtureListParameter(): Parameter(), m_impl(new Impl) {}

FixtureListParameter::FixtureListParameter(const QByteArray &t_id, const QString &t_name, QVector<FixtureParameterData> t_default, int connectionOptions):
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions),
    m_impl(new Impl)
{

}

FixtureListParameter::~FixtureListParameter()
{
    delete m_impl;
}

QWidget *FixtureListParameter::createWidget(keira::NodeEditor *item) const
{
    if(isReadOnly() || hasInput())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }

    auto selector = new SceneObjectSelector(photonApp->project()->sceneRoot());

    if(hasInput())
    {



        auto fixtureList = value().value<QVector<FixtureParameterData>>();

        QVector<SceneObject *> selected;
        for(const auto &data : fixtureList)
        {
            auto foundObj = SceneIterator::FindOne(photonApp->project()->sceneRoot(),[data](SceneObject *t_sceneObject){
                return t_sceneObject->uniqueId() == data.fixtureId;
            });
            if(foundObj)
                selected << foundObj;
        }
        selector->setSelectedObjects(selected);
    }
    else
    {
        QVector<SceneObject *> selected;
        for(auto sceneId : m_impl->selected)
        {
            auto sceneObj = photonApp->project()->scene()->findObjectById(sceneId);

            if(sceneObj)
                selected.append(sceneObj);
        }

        selector->setSelectedObjects(selected);
    }


    FixtureListParameter *param = const_cast<FixtureListParameter*>(this);

    selector->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));
    SceneObjectSelector::connect(selector, &SceneObjectSelector::selectionChanged,[selector, param](){param->setValue(param->updateValue(selector));});

    return selector;
}

void FixtureListParameter::updateWidget(QWidget *t_widget) const
{
    /*
    if(isReadOnly())
    {

    }
    else
    {
        ColorWheelSwatch *swatch = static_cast<ColorWheelSwatch*>(t_widget);
        swatch->setColor(value().value<QColor>());
    }
*/

}

QVariant FixtureListParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();

    QVector<FixtureParameterData> fixtureList;
    m_impl->selected.clear();
    auto selected = static_cast<SceneObjectSelector*>(t_widget)->selectedObjects();
    QSet<QByteArray> testSet;

    for(auto sceneObj : selected)
    {
        m_impl->selected.append(sceneObj->uniqueId());
        for(auto obj : SceneIterator::ToList(sceneObj))
        {
            if(obj->typeId() == "fixture")
            {
                FixtureParameterData data;
                data.fixtureId = static_cast<Fixture*>(obj)->uniqueId();

                if(!testSet.contains(data.fixtureId))
                {
                    fixtureList.append(data);
                    testSet << data.fixtureId;
                }
            }
        }
    }

    return QVariant::fromValue(fixtureList);
}


void FixtureListParameter::readFromJson(const QJsonObject &t_json)
{
    m_impl->selected.clear();
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


    QJsonArray selectedArray = t_json.value("selected").toArray();
    for(auto sceneObj : selectedArray)
    {
        m_impl->selected.append(sceneObj.toString().toLatin1());
    }
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

    QJsonArray objArray;
    for(auto sceneObj : m_impl->selected)
    {
        objArray.append(QString{sceneObj});
    }
    t_json.insert("selected",objArray);


    t_json.insert("value",fixArray);

}

} // namespace photon

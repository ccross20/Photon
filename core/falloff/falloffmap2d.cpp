#include <QTransform>
#include "falloffmap2d.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "scene/scenemanager.h"
#include "photoncore.h"
#include "project/project.h"


namespace photon {

class FalloffMap2D::Impl
{
public:
    void rebuildTransform();
    QVector<Falloff2DData> positions;
    QTransform transform;
    QPointF startPt;
    QPointF endPt;
};

void FalloffMap2D::Impl::rebuildTransform()
{
    transform.reset();

    QPointF delta = endPt - startPt;
    double length = QLineF(QPointF(), delta).length();
    transform.translate(startPt.x(), startPt.y());
    transform.scale(length, length);
    transform.rotate(qRadiansToDegrees(atan2(delta.y(), delta.x())));

    transform = transform.inverted();

}

FalloffMap2D::FalloffMap2D() : SceneFalloffMap(),m_impl(new Impl)
{
    m_impl->startPt = QPointF(0,.5);
    m_impl->endPt = QPointF(1,.5);
    m_impl->rebuildTransform();
}

FalloffMap2D::FalloffMap2D(const FalloffMap2D &other) : SceneFalloffMap(),m_impl(new Impl)
{
    setSceneObjectData(other.sceneObjectData());
    m_impl->startPt = other.startPosition();
    m_impl->endPt = other.endPosition();
    m_impl->rebuildTransform();
}

FalloffMap2D::~FalloffMap2D()
{
    delete m_impl;
}

FalloffMap2D &FalloffMap2D::operator=(const FalloffMap2D &other)
{
    setSceneObjectData(other.sceneObjectData());
    m_impl->startPt = other.startPosition();
    m_impl->endPt = other.endPosition();
    m_impl->rebuildTransform();
    return *this;
}

void FalloffMap2D::addSceneObject(SceneObject *t_fixture, const QPointF &t_position)
{
    Falloff2DData d;
    d.sceneObjectId = t_fixture->uniqueId();
    d.position = t_position;
    m_impl->positions.append(d);
}

void FalloffMap2D::removeSceneObject(SceneObject *t_fixture)
{
    for(auto it = m_impl->positions.cbegin(); it != m_impl->positions.cend(); ++it)
    {
        if((*it).sceneObjectId == t_fixture->uniqueId())
        {
            m_impl->positions.erase(it);
            return;
        }
    }
}

void FalloffMap2D::setSceneObjectPosition(SceneObject *t_fixture, const QPointF &t_position)
{
    for(auto &data : m_impl->positions)
    {
        if(data.sceneObjectId == t_fixture->uniqueId())
        {
            data.position = t_position;
        }
    }
}

QPointF FalloffMap2D::sceneObjectPosition(SceneObject *t_fixture) const
{
    for(const auto &data : m_impl->positions)
    {
        if(data.sceneObjectId == t_fixture->uniqueId())
        {
            return data.position;
        }
    }

    return QPointF{};
}

double FalloffMap2D::falloff(SceneObject *t_searchObj) const
{
    for(const auto &data : m_impl->positions)
    {
        auto sceneObj = photonApp->project()->scene()->findObjectById(data.sceneObjectId);

        auto foundObj = SceneIterator::FindOne(sceneObj,[t_searchObj](SceneObject *t_obj){return t_obj->uniqueId() == t_searchObj->uniqueId();});
        if(foundObj)
        {
            return std::max(std::min(m_impl->transform.map(data.position).x(), 1.0), 0.0);
        }
    }

    return 0;
}

QVector<FalloffMap2D::Falloff2DData> FalloffMap2D::sceneObjectData() const
{
    return m_impl->positions;
}

void FalloffMap2D::setSceneObjectData(const QVector<Falloff2DData> &t_positions)
{
    m_impl->positions = t_positions;
}

QPointF FalloffMap2D::startPosition() const
{
    return m_impl->startPt;
}

QPointF FalloffMap2D::endPosition() const
{
    return m_impl->endPt;
}

void FalloffMap2D::setStartPosition(const QPointF &t_pt)
{
    m_impl->startPt = t_pt;
    m_impl->rebuildTransform();
}

void FalloffMap2D::setEndPosition(const QPointF &t_pt)
{
    m_impl->endPt = t_pt;
    m_impl->rebuildTransform();
}

void FalloffMap2D::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("startPt"))
    {
        auto startPtObj = t_json.value("startPt").toObject();
        m_impl->startPt = QPointF(startPtObj.value("x").toDouble(0.0), startPtObj.value("y").toDouble(0.0));
    }

    if(t_json.contains("endPt"))
    {
        auto endPtObj = t_json.value("endPt").toObject();
        m_impl->endPt = QPointF(endPtObj.value("x").toDouble(0.0), endPtObj.value("y").toDouble(0.0));
    }

    auto fixtureArray = t_json.value("sceneObjects").toArray();
    for(auto fixData : fixtureArray)
    {
        auto fixObj = fixData.toObject();
        if(!fixObj.contains("pos"))
            continue;
        auto posObj = fixObj.value("pos").toObject();
        QPointF pos = QPointF(posObj.value("x").toDouble(0.0), posObj.value("y").toDouble(0.0));

        Falloff2DData d;
        d.sceneObjectId = fixObj.value("uniqueId").toString().toLatin1();
        d.position = pos;
        m_impl->positions.append(d);
    }
    m_impl->rebuildTransform();

}

QJsonObject pointToJson(const QPointF &t_pt)
{
    return QJsonObject{{"x",t_pt.x()},{"y",t_pt.y()}};
}

void FalloffMap2D::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("startPt", pointToJson(m_impl->startPt));
    t_json.insert("endPt", pointToJson(m_impl->endPt));

    QJsonArray array;
    for(const auto &fix : m_impl->positions)
    {
        array.append(QJsonObject{{"pos",pointToJson(fix.position)},{"uniqueId",QString(fix.sceneObjectId)}});
    }

    t_json.insert("sceneObjects", array);
}


} // namespace photon

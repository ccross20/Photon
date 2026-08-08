#include "cuelayer.h"

namespace photon {

class CueLayer::Impl
{
public:
    QList<float> markers;
    QString name;
    bool isSnappable = true;
    bool isVisible = true;
    bool isEditable = true;
    QColor color = Qt::red;
};

CueLayer::CueLayer(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}
CueLayer::~CueLayer()
{
    delete m_impl;
}

void CueLayer::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    emit metadataChanged(this);
}

QString CueLayer::name() const
{
    return m_impl->name;
}

void CueLayer::addMarkers(const QList<float> &t_markers)
{
    m_impl->markers.append(t_markers);
    emit markersChanged(this);
}

void CueLayer::replaceMarkers(const QList<float> &t_markers)
{
    m_impl->markers.clear();
    m_impl->markers.append(t_markers);
    emit markersChanged(this);
}

void CueLayer::insertMarker(float t_value)
{
    for(auto it = m_impl->markers.cbegin(); it != m_impl->markers.cend(); ++it)
    {
        if(*it > t_value)
        {
            m_impl->markers.insert(it, t_value);
            emit markersChanged(this);
            return;
        }
    }
    m_impl->markers.append(t_value);
    emit markersChanged(this);
}

void CueLayer::setIsVisible(bool t_value)
{
    if(m_impl->isVisible == t_value)
        return;
    m_impl->isVisible = t_value;
    emit metadataChanged(this);
}

bool CueLayer::isVisible() const
{
    return m_impl->isVisible;
}

void CueLayer::setIsSnappable(bool t_value)
{
    if(m_impl->isSnappable == t_value)
        return;
    m_impl->isSnappable = t_value;
    emit metadataChanged(this);
}

bool CueLayer::isSnappable() const
{
    return m_impl->isSnappable;
}

void CueLayer::setIsEditable(bool t_value)
{
    if(m_impl->isEditable == t_value)
        return;
    m_impl->isEditable = t_value;
    emit metadataChanged(this);
}

bool CueLayer::isEditable() const
{
    return m_impl->isEditable;
}

void CueLayer::setColor(const QColor &t_color)
{
    if(m_impl->color == t_color)
        return;
    m_impl->color = t_color;
    emit metadataChanged(this);
}

QColor CueLayer::color() const
{
    return m_impl->color;
}

bool CueLayer::snapToMarker(float time, float *outTime, float tolerance) const
{
    bool foundSnap = false;
    float winner = 10000000.f;
    for(auto it = m_impl->markers.cbegin(); it != m_impl->markers.cend(); ++it)
    {
        if(abs(*it - time) < tolerance)
        {
            if(abs(*it - time) < abs(winner-time))
                winner = *it;
            else
                return true;
            *outTime = winner;
            foundSnap = true;
        }
    }



    *outTime = time;
    return foundSnap;
}

const QList<float> &CueLayer::markers() const
{
    return m_impl->markers;
}


void CueLayer::sort()
{
    std::sort(m_impl->markers.begin(), m_impl->markers.end(),[](const float &a, const float &b){return a < b;});
}

void CueLayer::readFromJson(const QJsonObject &t_json, const LoadContext &)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->isSnappable = t_json.value("isSnappable").toBool(m_impl->isSnappable);
    m_impl->isVisible = t_json.value("isVisible").toBool(m_impl->isVisible);
    m_impl->isEditable = t_json.value("isEditable").toBool(m_impl->isEditable);
    m_impl->color = QColor(t_json.value("color").toString(m_impl->color.name()));

    auto array = t_json.value("markers").toArray();
    m_impl->markers.clear();

    for(auto marker : array)
    {
        m_impl->markers.append(marker.toDouble());
    }
}

void CueLayer::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("isSnappable", m_impl->isSnappable);
    t_json.insert("isVisible", m_impl->isVisible);
    t_json.insert("isEditable", m_impl->isEditable);
    t_json.insert("color", m_impl->color.name());

    QJsonArray array;

    for(auto marker : m_impl->markers)
    {
        array.append(marker);
    }
    t_json.insert("markers", array);
}

} // namespace photon

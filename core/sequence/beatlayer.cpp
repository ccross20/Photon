#include "beatlayer.h"

namespace photon {

class BeatLayer::Impl
{
public:
    QList<float> beats;
    QString name;
    bool isSnappable = true;
    bool isVisible = true;
    bool isEditable = true;
};

BeatLayer::BeatLayer(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}
BeatLayer::~BeatLayer()
{
    delete m_impl;
}

void BeatLayer::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    emit metadataChanged(this);
}

QString BeatLayer::name() const
{
    return m_impl->name;
}

void BeatLayer::addBeats(const QList<float> &t_beats)
{
    m_impl->beats.append(t_beats);
    emit beatsChanged(this);
}

void BeatLayer::replaceBeats(const QList<float> &t_beats)
{
    m_impl->beats.clear();
    m_impl->beats.append(t_beats);
    emit beatsChanged(this);
}

void BeatLayer::insertBeat(float t_value)
{
    for(auto it = m_impl->beats.cbegin(); it != m_impl->beats.cend(); ++it)
    {
        if(*it > t_value)
        {
            m_impl->beats.insert(it, t_value);
            emit beatsChanged(this);
            return;
        }
    }
    m_impl->beats.append(t_value);
}

void BeatLayer::setIsVisible(bool t_value)
{
    if(m_impl->isVisible == t_value)
        return;
    m_impl->isVisible = t_value;
    emit metadataChanged(this);
}

bool BeatLayer::isVisible() const
{
    return m_impl->isVisible;
}

void BeatLayer::setIsSnappable(bool t_value)
{
    if(m_impl->isSnappable == t_value)
        return;
    m_impl->isSnappable = t_value;
    emit metadataChanged(this);
}

bool BeatLayer::isSnappable() const
{
    return m_impl->isSnappable;
}

void BeatLayer::setIsEditable(bool t_value)
{
    if(m_impl->isEditable == t_value)
        return;
    m_impl->isEditable = t_value;
    emit metadataChanged(this);
}

bool BeatLayer::isEditable() const
{
    return m_impl->isEditable;
}

bool BeatLayer::snapToBeat(float time, float *outTime, float tolerance) const
{
    bool foundSnap = false;
    float winner = 10000000.f;
    for(auto it = m_impl->beats.cbegin(); it != m_impl->beats.cend(); ++it)
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

const QList<float> &BeatLayer::beats() const
{
    return m_impl->beats;
}


void BeatLayer::sort()
{
    std::sort(m_impl->beats.begin(), m_impl->beats.end(),[](const float &a, const float &b){return a < b;});
}

void BeatLayer::readFromJson(const QJsonObject &t_json, const LoadContext &)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->isSnappable = t_json.value("isSnappable").toBool(m_impl->isSnappable);
    m_impl->isVisible = t_json.value("isVisible").toBool(m_impl->isVisible);
    m_impl->isEditable = t_json.value("isEditable").toBool(m_impl->isEditable);

    auto array = t_json.value("beats").toArray();
    m_impl->beats.clear();

    for(auto beat : array)
    {
        m_impl->beats.append(beat.toDouble());
    }
}

void BeatLayer::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("isSnappable", m_impl->isSnappable);
    t_json.insert("isVisible", m_impl->isVisible);
    t_json.insert("isEditable", m_impl->isEditable);

    QJsonArray array;

    for(auto beat : m_impl->beats)
    {
        array.append(beat);
    }
    t_json.insert("beats", array);
}

} // namespace photon

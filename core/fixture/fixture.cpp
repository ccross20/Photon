#include "fixture.h"
#include <QColor>
#include <QJsonDocument>
#include <QFile>
#include "fixturechannel_p.h"
#include "fixturevirtualchannel.h"
#include "fixtureeditorwidget.h"
#include "fixturewheel.h"

namespace photon {

class Fixture::Impl
{
public:
    Fixture::Physical physical;
    QVector<FixtureChannel*> channels;
    QVector<FixtureVirtualChannel*> virtualChannels;
    QVector<FixtureMode> modes;
    QVector<FixtureWheel*> wheels;
    QString definitionPath;
    QString description;
    QString manufacturer;
    QString comments;
    QString identifier;
    int dmxOffset = 0;
    int dmxSize = 0;
    int universe = 1;
    int selectedMode = -1;
    int uniqueIndex = 0;
};

const QByteArray Fixture::FixtureMime = "application/x-photonfixture";

Fixture::Fixture(const QString &path) :SceneObject("fixture"), m_impl(new Impl)
{
    if(!path.isEmpty())
        loadFixtureDefinition(path);
}

Fixture::~Fixture()
{
    delete m_impl;
}

QWidget *Fixture::createEditor()
{
    auto editor = new FixtureEditorWidget;
    editor->setFixtures(QVector<Fixture*>{this});
    return editor;
}

const QVector<FixtureWheel*> &Fixture::wheels() const
{
    return m_impl->wheels;
}

QVector<FixtureCapability*> Fixture::findCapability(CapabilityType t_type, int t_index) const
{
    QVector<FixtureCapability*> results;

    int channelCounter = 0;
    for(auto it = m_impl->channels.cbegin(); it != m_impl->channels.cend(); ++it)
    {
        auto channel = *it;

        if(channel->capabilityType() == t_type)
        {
            if(t_index < 0 || channelCounter == t_index)
            {
                for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
                {
                    if((*capabilityIt)->type() == t_type)
                        results.append(*capabilityIt);
                }
            }

            if(channelCounter == t_index)
                return results;

            ++channelCounter;
        }
    }

    for(auto it = m_impl->virtualChannels.cbegin(); it != m_impl->virtualChannels.cend(); ++it)
    {
        auto channel = *it;

        if(channel->capabilityType() == t_type)
        {
            if(t_index < 0 || channelCounter == t_index)
            {
                for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
                {
                    if((*capabilityIt)->type() == t_type)
                        results.append(*capabilityIt);
                }
            }

            if(channelCounter == t_index)
                return results;

            ++channelCounter;
        }
    }

    return results;
}

void Fixture::setComments(const QString &t_value)
{
    m_impl->comments = t_value;
    emit metadataChanged(this);
}

void Fixture::setIdentifier(const QString &t_value)
{
    m_impl->identifier = t_value;
    emit metadataChanged(this);
}

QString Fixture::description() const
{
    return m_impl->description;
}

QString Fixture::manufacturer() const
{
    return m_impl->manufacturer;
}

QString Fixture::comments() const
{
    return m_impl->comments;
}

QString Fixture::identifier() const
{
    return m_impl->identifier;
}

int Fixture::dmxSize() const
{
    return m_impl->dmxSize;
}

int Fixture::universe() const
{
    return m_impl->universe;
}

int Fixture::uniqueIndex() const
{
    return m_impl->uniqueIndex;
}

void Fixture::setUniqueIndex(int t_value)
{
    m_impl->uniqueIndex = t_value;
}

void Fixture::setUniverse(int t_universe)
{
    m_impl->universe = t_universe;
    emit metadataChanged(this);
}

void Fixture::setDMXOffset(int t_offset)
{
    m_impl->dmxOffset = t_offset;
    emit metadataChanged(this);
}

int Fixture::dmxOffset() const
{
    return m_impl->dmxOffset;
}

const QVector<FixtureChannel*> &Fixture::channels() const
{
    return m_impl->channels;
}

FixtureChannel* Fixture::findChannelWithName(const QString &t_name) const
{
    auto result = std::find_if(m_impl->channels.cbegin(), m_impl->channels.cend(),[t_name](const FixtureChannel *t_testChannel){
                     return t_testChannel->name() == t_name;
                 });
    if(result != m_impl->channels.cend())
        return *result;
    return nullptr;
}

Fixture::Physical Fixture::physical() const
{
    return m_impl->physical;
}

void Fixture::loadFixtureDefinition(const QString &t_path)
{
    QFile loadFile(t_path);

    if (!loadFile.open(QIODevice::ReadOnly)) {
             qWarning("Couldn't open load file.");
             return;
         }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    m_impl->definitionPath = t_path;

    readFromOpenFixtureJson(loadDoc.object());
}

void Fixture::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    if(name().isEmpty())
        setName(t_json.value("name").toString(name()));
    if(t_json.contains("physical"))
    {
        auto physicalObj = t_json.value("physical").toObject();
        if(physicalObj.contains("dimensions"))
        {
            auto dimArray = physicalObj.value("dimensions").toArray();
            if(dimArray.size() == 3)
                m_impl->physical.dimensions = QVector3D{static_cast<float>(dimArray[0].toDouble()), static_cast<float>(dimArray[1].toDouble()), static_cast<float>(dimArray[2].toDouble())};
        }
        m_impl->physical.weight = physicalObj.value("weight").toDouble(0);
        m_impl->physical.power = physicalObj.value("power").toDouble(0);

        if(physicalObj.contains("DMXconnector"))
        {
            QString connectorString = physicalObj.value("DMXconnector").toString();
            if(connectorString == "3-pin")
                m_impl->physical.dmxConnector = Connector_3Pin;
            else if(connectorString == "3-pin (swapped +/-)")
                m_impl->physical.dmxConnector = Connector_3PinSwapped;
            else if(connectorString == "3-pin XLR IP65")
                m_impl->physical.dmxConnector = Connector_3PinXLRIP65;
            else if(connectorString == "5-pin")
                m_impl->physical.dmxConnector = Connector_5Pin;
            else if(connectorString == "5-pin XLR IP65")
                m_impl->physical.dmxConnector = Connector_5PinXLRIP65;
            else if(connectorString == "3-pin and 5-pin")
                m_impl->physical.dmxConnector = Connector_3PinAnd5Pin;
            else if(connectorString == "3.5mm stereo jack")
                m_impl->physical.dmxConnector = Connector_3_5_StereoJack;
        }

        if(physicalObj.contains("bulb"))
        {
            auto bulbObj = physicalObj.value("bulb").toObject();
            m_impl->physical.colorTemperature = bulbObj.value("colorTemperature").toDouble(0);
            m_impl->physical.lumens = bulbObj.value("lumens").toDouble(0);
        }

        if(physicalObj.contains("lens"))
        {
            auto lensArray = physicalObj.value("lens").toObject().value("degreesMinMax").toArray();
            if(lensArray.size() == 2)
            {
                m_impl->physical.lensMinimum = lensArray[0].toDouble(0);
                m_impl->physical.lensMaximum = lensArray[1].toDouble(0);
            }
        }

        if(physicalObj.contains("matrixPixels"))
        {
            auto pixelObj = physicalObj.value("matrixPixels").toObject();
            if(pixelObj.contains("dimensions"))
            {
                auto dimensions = pixelObj.value("dimensions").toArray();
                if(dimensions.size() == 3)
                {
                    m_impl->physical.matrixPixelDimensions = QVector3D{static_cast<float>(dimensions[0].toDouble()), static_cast<float>(dimensions[1].toDouble()), static_cast<float>(dimensions[2].toDouble())};
                }
            }

            if(pixelObj.contains("spacing"))
            {
                auto spacing = pixelObj.value("spacing").toArray();
                if(spacing.size() == 3)
                {
                    m_impl->physical.matrixPixelSpacing = QVector3D{static_cast<float>(spacing[0].toDouble()), static_cast<float>(spacing[1].toDouble()), static_cast<float>(spacing[2].toDouble())};
                }
            }
        }
    }

    if(t_json.contains("wheels"))
    {
        auto wheelObj = t_json.value("wheels").toObject();

        for(auto it = wheelObj.constBegin(); it != wheelObj.constEnd(); ++it)
        {
            //qDebug() << "add wheel" << it.key();
            auto wheel = new FixtureWheel(it.key());
            wheel->readFromOpenFixtureJson(it.value().toObject());
            m_impl->wheels << wheel;
        }
    }

    if(t_json.contains("availableChannels"))
    {
        auto channels = t_json.value("availableChannels").toObject();
        QVector<FixtureChannel*> colorChannels;


        for(auto it = channels.constBegin(); it != channels.constEnd(); ++it)
        {
            auto channelObj = it.value().toObject();

            FixtureChannel *fixtureChannel = new FixtureChannel(it.key(), 0);
            fixtureChannel->setFixture(this);
            fixtureChannel->readFromOpenFixtureJson(channelObj);
            fixtureChannel->m_impl->channelNumber = -1;
            fixtureChannel->m_impl->globalChannelNumber = 0;
            m_impl->channels.append(fixtureChannel);

            for(const auto &alias : fixtureChannel->m_impl->fineChannelsAliases)
            {
                FixtureChannel *fineChannel = new FixtureChannel(alias, 0);
                fineChannel->setFixture(this);
                fineChannel->m_impl->channelNumber = -1;
                fineChannel->m_impl->globalChannelNumber = 0;
                m_impl->channels.append(fineChannel);
                fixtureChannel->m_impl->fineChannels.append(fineChannel);
            }


            if(fixtureChannel->capabilityType() == Capability_Cyan ||
                    fixtureChannel->capabilityType() == Capability_Magenta ||
                    fixtureChannel->capabilityType() == Capability_Yellow)
            {
                colorChannels.append(fixtureChannel);

                if(colorChannels.length() == 3)
                {
                    auto vChannel = new FixtureVirtualChannel(colorChannels);
                    vChannel->setFixture(this);
                    m_impl->virtualChannels.append(vChannel);
                    colorChannels.clear();
                }
            }


        }

    }

    if(t_json.contains("modes"))
    {
        auto modes = t_json.value("modes").toArray();

        for(auto it = modes.constBegin(); it != modes.constEnd(); ++it)
        {
            auto modeObj = (*it).toObject();

            FixtureMode mode;
            mode.name = modeObj.value("name").toString("");
            mode.shortName = modeObj.value("shortName").toString("");

            if(modeObj.contains("channels"))
            {
                auto channelArray = modeObj.value("channels").toArray();

                for(auto channelIt = channelArray.constBegin(); channelIt != channelArray.constEnd(); ++channelIt)
                {
                    mode.channels.append((*channelIt).toString());
                }
            }
            m_impl->modes.append(mode);

        }
    }

    setMode(0);
}

QVector<FixtureMode> Fixture::modes() const
{
    return m_impl->modes;
}

void Fixture::setMode(uchar t_mode)
{
    if(m_impl->selectedMode == t_mode)
        return;

    for(auto it = m_impl->channels.cbegin(); it != m_impl->channels.cend(); ++it)
        (*it)->setChannelNumber(-1);

    if(t_mode < 0 || t_mode >= m_impl->modes.length())
    {
        m_impl->selectedMode = -1;
        return;
    }

    m_impl->selectedMode = t_mode;

    const FixtureMode &mode = m_impl->modes[m_impl->selectedMode];
    m_impl->dmxSize = mode.channels.length();

    uchar counter = 0;
    for(const auto &channelName : mode.channels)
    {
        auto channel = findChannelWithName(channelName);
        if(channel)
        {
            channel->setChannelNumber(counter++);
        }
        else
        {
            qDebug() << "Could not find:" << channelName;
        }
    }
    emit metadataChanged(this);
}

int Fixture::mode() const
{
    return m_impl->selectedMode;
}

void Fixture::readFromJson(const QJsonObject &json)
{
    SceneObject::readFromJson(json);
    //QString version = json.value("version").toString();
    m_impl->description = json.value("description").toString();
    m_impl->dmxOffset = json.value("dmxOffset").toInt(0);
    m_impl->dmxSize = json.value("dmxSize").toInt(0);
    m_impl->universe = json.value("universe").toInt(1);
    m_impl->manufacturer = json.value("manufacturer").toString();
    m_impl->comments = json.value("comments").toString();
    m_impl->identifier = json.value("identifier").toString();
    m_impl->definitionPath = json.value("definitionPath").toString();
    m_impl->uniqueIndex = json.value("uniqueIndex").toInt(0);
    loadFixtureDefinition(m_impl->definitionPath);
    setMode(json.value("selectedMode").toInt(-1));

}

void Fixture::writeToJson(QJsonObject &json) const
{
    SceneObject::writeToJson(json);
    json.insert("version", "1.0");
    json.insert("description", m_impl->description);
    json.insert("dmxOffset", m_impl->dmxOffset);
    json.insert("dmxSize", m_impl->dmxSize);
    json.insert("universe", m_impl->universe);
    json.insert("manufacturer", m_impl->manufacturer);
    json.insert("comments", m_impl->comments);
    json.insert("identifier", m_impl->identifier);
    json.insert("selectedMode", m_impl->selectedMode);
    json.insert("definitionPath", m_impl->definitionPath);
    json.insert("uniqueIndex", m_impl->uniqueIndex);


}

} // namespace photon

#include "fixture.h"
#include <QColor>
#include <QJsonDocument>
#include <QFile>
#include <QMutex>
#include <atomic>
#include "fixturechannel_p.h"
#include "fixturevirtualchannel.h"
#include "fixtureeditorwidget.h"
#include "capability/colorcapability.h"
#include "capability/fixturecolorcalibration.h"
#include "capability/anglecapability.h"
#include "fixturewheel.h"
#include "fixturelibrary.h"
#include "project/project.h"
#include "photoncore.h"

namespace photon {

class Fixture::Impl
{
public:
    Fixture::Physical physical;
    QVector<FixtureChannel*> channels;
    QVector<FixtureVirtualChannel*> virtualChannels;
    QVector<ColorCapability*> colors;
    QVector<FixtureMode> modes;
    QVector<FixtureWheel*> wheels;
    QString definitionPath;
    QString description;
    QString manufacturer;
    QString comments;
    QString identifier;
    QStringList categories;   // from the OpenFixture definition
    QString modelType;        // per-fixture model override ("" = auto from category)
    QString beamStyle;        // per-fixture beam style override ("" = follow global toggle)
    float panOffset = 0.0f;   // degrees added to the pan channel before aiming (mounting calibration)
    float tiltOffset = 0.0f;  // same, for tilt
    bool panFlip = false;     // reverses the pan channel's direction of travel (real DMX)
    bool tiltFlip = false;    // same, for tilt
    bool panInvert = false;   // reverses the visualiser's pan preview only (no DMX effect)
    bool tiltInvert = false;  // same, for tilt
    int dmxOffset = 0;
    int dmxSize = 0;
    int universe = 1;
    int selectedMode = -1;
    int uniqueIndex = 0;

    // Read by OutputOverridesNode on the eval thread while the setup dialog
    // edits them on the main thread.
    std::atomic<bool> laserArmed = false;
    std::atomic<bool> laserSetupActive = false;
    mutable QMutex laserSetupMutex;
    Fixture::LaserSetup laserSetup;
    QString laserPreviewFolder;
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

bool Fixture::isLaser() const
{
    return m_impl->categories.contains("Laser", Qt::CaseInsensitive);
}

bool Fixture::isLaserArmed() const
{
    return m_impl->laserArmed;
}

void Fixture::setLaserArmed(bool t_armed)
{
    if(m_impl->laserArmed.exchange(t_armed) == t_armed)
        return;
    emit metadataChanged(this);
}

bool Fixture::isLaserSetupActive() const
{
    return m_impl->laserSetupActive;
}

void Fixture::setLaserSetupActive(bool t_active)
{
    if(m_impl->laserSetupActive.exchange(t_active) == t_active)
        return;
    emit metadataChanged(this);
}

Fixture::LaserSetup Fixture::laserSetup() const
{
    QMutexLocker lock(&m_impl->laserSetupMutex);
    return m_impl->laserSetup;
}

void Fixture::setLaserSetup(const LaserSetup &t_setup)
{
    {
        QMutexLocker lock(&m_impl->laserSetupMutex);
        if(m_impl->laserSetup == t_setup)
            return;
        m_impl->laserSetup = t_setup;
    }
    emit metadataChanged(this);
}

QString Fixture::laserPreviewFolder() const
{
    return m_impl->laserPreviewFolder;
}

void Fixture::setLaserPreviewFolder(const QString &t_folder)
{
    if(m_impl->laserPreviewFolder == t_folder)
        return;
    m_impl->laserPreviewFolder = t_folder;
    emit metadataChanged(this);
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

FixtureWheel *Fixture::findWheel(const QString &t_name) const
{
    QString searchName = t_name.toLower();
    for(auto it = m_impl->wheels.cbegin(); it != m_impl->wheels.cend(); ++it)
    {
        if((*it)->name() == searchName)
            return *it;
    }
    return nullptr;
}

QString Fixture::resolveWheelName(const QString &t_role) const
{
    const QString role = t_role.toLower();

    // Which slot type this role targets.
    FixtureWheelSlot::SlotType want = FixtureWheelSlot::Slot_Gobo;
    if (role.contains("color"))
        want = FixtureWheelSlot::Slot_Color;
    else if (role.contains("animation"))
        want = FixtureWheelSlot::Slot_AnimationGoboStart;

    // Trailing ordinal (1-based), e.g. "gobo wheel 2" -> 2; default 1.
    int ordinal = 1;
    for (int i = role.size() - 1; i >= 0; --i) {
        if (role[i].isDigit()) {
            int j = i;
            while (j > 0 && role[j - 1].isDigit()) --j;
            ordinal = role.mid(j, i - j + 1).toInt();
            break;
        }
    }

    int matchCount = 0;
    for (FixtureWheel *wheel : m_impl->wheels) {
        bool isMatch = false;
        for (FixtureWheelSlot *s : wheel->allSlots()) {
            if (s && s->type() == want) { isMatch = true; break; }
        }
        if (isMatch && ++matchCount == ordinal)
            return wheel->name();
    }
    return QString();
}

QVector<FixtureCapability*> Fixture::findCapability(CapabilityType t_type, int t_index) const
{
    QVector<FixtureCapability*> results;

    int channelCounter = 0;
    for(auto it = m_impl->channels.cbegin(); it != m_impl->channels.cend(); ++it)
    {
        auto channel = *it;

        // Skip channels the selected mode doesn't map to a DMX slot: their
        // channelNumber is -1, so reading/writing their capabilities either
        // no-ops or aliases onto slot 0. e.g. a fixture with a Dimmer channel
        // only in its 10-channel mode must fall back to full intensity when
        // patched in a 6-channel mode that has none.
        if(!channel->isValid())
            continue;

        //if(channel->capabilityType() == t_type)
        {
            //if(t_index < 0 || channelCounter == t_index)
            {
                for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
                {
                    if((*capabilityIt)->type() == t_type)
                        results.append(*capabilityIt);
                }
            }
/*
            if(channelCounter == t_index)
                return results;
*/

            ++channelCounter;
        }
    }

    for(auto it = m_impl->virtualChannels.cbegin(); it != m_impl->virtualChannels.cend(); ++it)
    {
        auto channel = *it;

        if(!channel->isValid())
            continue;

        //if(channel->capabilityType() == t_type)
        {
            //if(t_index < 0 || channelCounter == t_index)
            {
                for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
                {
                    if((*capabilityIt)->type() == t_type)
                        results.append(*capabilityIt);
                }
            }
/*
            if(channelCounter == t_index)
                return results;
*/

            ++channelCounter;
        }
    }

    return results;
}


QVector<FixtureCapability*> Fixture::findCapability(CapabilityType t_type, const QString &t_name) const
{

    QVector<FixtureCapability*> results;

    for(auto it = m_impl->channels.cbegin(); it != m_impl->channels.cend(); ++it)
    {
        auto channel = *it;

        // See findCapability(type, index): channels outside the selected mode
        // aren't part of the fixture's live DMX footprint.
        if(!channel->isValid())
            continue;

        if(channel->name().toLower() == t_name.toLower())
        {
            for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
            {
                if((*capabilityIt)->type() == t_type)
                    results.append(*capabilityIt);
            }
        }

    }

    for(auto it = m_impl->virtualChannels.cbegin(); it != m_impl->virtualChannels.cend(); ++it)
    {
        auto channel = *it;

        if(!channel->isValid())
            continue;

        if(channel->name().toLower() == t_name.toLower())
        {
            for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
            {
                if((*capabilityIt)->type() == t_type)
                    results.append(*capabilityIt);
            }
        }

    }

    return results;
}

QStringList Fixture::channelNamesForCapability(CapabilityType t_type) const
{
    QStringList results;

    for(auto it = m_impl->channels.cbegin(); it != m_impl->channels.cend(); ++it)
    {
        auto channel = *it;
        for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
        {
            if((*capabilityIt)->type() == t_type)
            {
                if(!results.contains(channel->name(), Qt::CaseInsensitive))
                    results.append(channel->name());
                break;
            }
        }
    }

    for(auto it = m_impl->virtualChannels.cbegin(); it != m_impl->virtualChannels.cend(); ++it)
    {
        auto channel = *it;
        for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
        {
            if((*capabilityIt)->type() == t_type)
            {
                if(!results.contains(channel->name(), Qt::CaseInsensitive))
                    results.append(channel->name());
                break;
            }
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

QStringList Fixture::categories() const
{
    return m_impl->categories;
}

QString Fixture::modelType() const
{
    return m_impl->modelType;
}

void Fixture::setModelType(const QString &t_value)
{
    if(m_impl->modelType == t_value)
        return;
    m_impl->modelType = t_value;
    emit metadataChanged(this);
}

QString Fixture::beamStyle() const
{
    return m_impl->beamStyle;
}

void Fixture::setBeamStyle(const QString &t_value)
{
    if(m_impl->beamStyle == t_value)
        return;
    m_impl->beamStyle = t_value;
    emit metadataChanged(this);
}

float Fixture::panOffset() const
{
    return m_impl->panOffset;
}

void Fixture::setPanOffset(float t_value)
{
    // qFuzzyCompare is unreliable near zero (its tolerance scales with the
    // values being compared), and 0 is this property's common/default value.
    if(qAbs(m_impl->panOffset - t_value) < 0.0001f)
        return;
    m_impl->panOffset = t_value;
    emit metadataChanged(this);
}

float Fixture::tiltOffset() const
{
    return m_impl->tiltOffset;
}

void Fixture::setTiltOffset(float t_value)
{
    if(qAbs(m_impl->tiltOffset - t_value) < 0.0001f)
        return;
    m_impl->tiltOffset = t_value;
    emit metadataChanged(this);
}

bool Fixture::panFlip() const
{
    return m_impl->panFlip;
}

void Fixture::setPanFlip(bool t_value)
{
    if(m_impl->panFlip == t_value)
        return;
    m_impl->panFlip = t_value;
    emit metadataChanged(this);
}

bool Fixture::tiltFlip() const
{
    return m_impl->tiltFlip;
}

void Fixture::setTiltFlip(bool t_value)
{
    if(m_impl->tiltFlip == t_value)
        return;
    m_impl->tiltFlip = t_value;
    emit metadataChanged(this);
}

bool Fixture::panInvert() const
{
    return m_impl->panInvert;
}

void Fixture::setPanInvert(bool t_value)
{
    if(m_impl->panInvert == t_value)
        return;
    m_impl->panInvert = t_value;
    emit metadataChanged(this);
}

bool Fixture::tiltInvert() const
{
    return m_impl->tiltInvert;
}

void Fixture::setTiltInvert(bool t_value)
{
    if(m_impl->tiltInvert == t_value)
        return;
    m_impl->tiltInvert = t_value;
    emit metadataChanged(this);
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

int Fixture::colorCount() const
{
    return m_impl->colors.length();
}

ColorCapability *Fixture::colorAtIndex(int index) const
{
    return m_impl->colors[abs(index%(m_impl->colors.length()))];
}

ColorCapability *Fixture::color() const
{
    return colorCount() > 0 ? colorAtIndex(0) : nullptr;
}

AngleCapability *Fixture::pan() const
{
    return static_cast<AngleCapability *>(findCapability(Capability_Pan).value(0));
}

AngleCapability *Fixture::tilt() const
{
    return static_cast<AngleCapability *>(findCapability(Capability_Tilt).value(0));
}

AngleCapability *Fixture::zoom() const
{
    return static_cast<AngleCapability *>(findCapability(Capability_Zoom).value(0));
}

AngleCapability *Fixture::focus() const
{
    return static_cast<AngleCapability *>(findCapability(Capability_Focus).value(0));
}

bool extractRange(QString text, int *start, int *end, QString *prefix)
{
    int startIndex = text.indexOf("[");
    int endIndex = text.indexOf("]")-1;
    *prefix = text;
    bool hasRange = startIndex >= 0 && endIndex >= 0;

    if(hasRange)
    {
        *prefix = text.left(startIndex);
        QString range = text.mid(startIndex+1,endIndex - startIndex);
        auto rangeList = range.split("-");
        *start = rangeList[0].toInt();
        *end = rangeList[1].toInt();
        return true;
    }
    return false;
}

QString Fixture::definitionPath() const
{
    return m_impl->definitionPath;
}

void Fixture::loadFixtureDefinition(const QString &t_path)
{
    QFile loadFile(t_path);

    if (!loadFile.open(QIODevice::ReadOnly)) {

        QFileInfo info(t_path);
        loadFile.setFileName(FixtureLibrary::defaultDirectory() + "/" + info.fileName());

        if (!loadFile.open(QIODevice::ReadOnly)) {

             qWarning() << "Couldn't open fixture file " << t_path;
             return;
        }
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

    if(t_json.contains("categories"))
    {
        m_impl->categories.clear();
        for(const auto &c : t_json.value("categories").toArray())
            m_impl->categories.append(c.toString());
    }

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

        for(auto it = channels.constBegin(); it != channels.constEnd(); ++it)
        {
            auto channelObj = it.value().toObject();
            auto key = it.key();
            int rangeStart = 0;
            int rangeEnd = 0;
            QString prefix = key;
            bool hasRange = extractRange(key, &rangeStart, &rangeEnd, &prefix);

            for(int i = rangeStart; i <= rangeEnd; ++i)
            {
                QString channelName = prefix;
                if(hasRange)
                    channelName = prefix + QString::number(i);

                FixtureChannel *fixtureChannel = new FixtureChannel(channelName, 0);
                fixtureChannel->setFixture(this);
                fixtureChannel->readFromOpenFixtureJson(channelObj);
                fixtureChannel->m_impl->channelNumber = -1;
                fixtureChannel->m_impl->globalChannelNumber = 0;
                fixtureChannel->setFixture(this);
                m_impl->channels.append(fixtureChannel);

                for(const auto &alias : fixtureChannel->m_impl->fineChannelsAliases)
                {
                    FixtureChannel *fineChannel = new FixtureChannel(alias, 0);
                    fineChannel->setFixture(this);
                    fineChannel->m_impl->channelNumber = -1;
                    fineChannel->m_impl->globalChannelNumber = 0;
                    fineChannel->setFixture(this);
                    m_impl->channels.append(fineChannel);
                    fixtureChannel->m_impl->fineChannels.append(fineChannel);
                }
            }



/*
            if(fixtureChannel->capabilityType() == Capability_Cyan ||
                    fixtureChannel->capabilityType() == Capability_Magenta ||
                    fixtureChannel->capabilityType() == Capability_Yellow ||
                fixtureChannel->capabilityType() == Capability_Red ||
                fixtureChannel->capabilityType() == Capability_Green ||
                fixtureChannel->capabilityType() == Capability_Blue ||
                fixtureChannel->capabilityType() == Capability_Amber ||
                fixtureChannel->capabilityType() == Capability_Indigo ||
                fixtureChannel->capabilityType() == Capability_White ||
                fixtureChannel->capabilityType() == Capability_UV)
            {
                colorChannels.append(fixtureChannel);
            }
*/
        }


    }

    if(t_json.contains("virtual"))
    {
        auto virtualArray = t_json.value("virtual").toArray();

        for(auto it = virtualArray.constBegin(); it != virtualArray.constEnd(); ++it)
        {
            //qDebug() << "add wheel" << it.key();
            auto virtualObj = (*it).toObject();

            auto name = virtualObj.value("name").toString();
            auto vArray = virtualObj.value("channels").toArray();


            int rangeStart = 0;
            int rangeEnd = 0;
            QString prefix = name;
            bool hasRange = extractRange(name, &rangeStart, &rangeEnd, &prefix);
            if(hasRange)
            {
                for(int i = rangeStart; i<= rangeEnd; ++i)
                {
                    QVector<FixtureChannel*> colorChannels;

                    for(auto capIt = vArray.constBegin(); capIt != vArray.constEnd(); ++capIt)
                    {
                        auto foundChannel = findChannelWithName((*capIt).toString() + QString::number(i));

                        if(foundChannel)
                            colorChannels.append(foundChannel);
                        else
                            qDebug() << "[Virtual Channel] Could not find channel: " << (*capIt).toString() + QString::number(i);
                    }

                    auto vChannel = new FixtureVirtualChannel(colorChannels, prefix + QString::number(i));
                    vChannel->setFixture(this);
                    m_impl->virtualChannels.append(vChannel);
                    qDebug() << "Create Virtual channel";
                }

            }
            else
            {
                QVector<FixtureChannel*> colorChannels;

                for(auto capIt = vArray.constBegin(); capIt != vArray.constEnd(); ++capIt)
                {
                    auto foundChannel = findChannelWithName((*capIt).toString());

                    if(foundChannel)
                        colorChannels.append(foundChannel);
                    else
                        qDebug() << "[Virtual Channel] Could not find channel: " << (*capIt).toString();
                }

                auto vChannel = new FixtureVirtualChannel(colorChannels, name);
                vChannel->setFixture(this);
                m_impl->virtualChannels.append(vChannel);
                qDebug() << "Create Virtual channel";
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

    m_impl->colors.clear();

    for(auto *vchannel : m_impl->virtualChannels)
    {
        for(auto *capability : vchannel->capabilities())
        {
            ColorCapability *colorCap = dynamic_cast<ColorCapability*>(capability);
            if(colorCap)
            {
                colorCap->setIndex(m_impl->colors.length());
                m_impl->colors.append(colorCap);
            }
        }
    }

    if(!m_impl->definitionPath.isEmpty() && !m_impl->colors.isEmpty())
    {
        const FixtureColorCalibration calibration = FixtureColorCalibrationStore::load(m_impl->definitionPath);
        if(calibration.isValid())
            for(auto *colorCap : m_impl->colors)
                colorCap->setCalibration(calibration);
    }

    emit metadataChanged(this);
}

int Fixture::mode() const
{
    return m_impl->selectedMode;
}

void Fixture::readFromJson(const QJsonObject &json, const LoadContext &t_context)
{
    SceneObject::readFromJson(json, t_context);
    //QString version = json.value("version").toString();
    m_impl->description = json.value("description").toString();
    m_impl->dmxOffset = json.value("dmxOffset").toInt(0);
    m_impl->dmxSize = json.value("dmxSize").toInt(0);
    m_impl->universe = json.value("universe").toInt(1);
    m_impl->manufacturer = json.value("manufacturer").toString();
    m_impl->comments = json.value("comments").toString();
    m_impl->modelType = json.value("modelType").toString();
    m_impl->beamStyle = json.value("beamStyle").toString();
    m_impl->panOffset = float(json.value("panOffset").toDouble(0.0));
    m_impl->tiltOffset = float(json.value("tiltOffset").toDouble(0.0));
    m_impl->panFlip = json.value("panFlip").toBool(false);
    m_impl->tiltFlip = json.value("tiltFlip").toBool(false);
    m_impl->panInvert = json.value("panInvert").toBool(false);
    m_impl->tiltInvert = json.value("tiltInvert").toBool(false);
    m_impl->identifier = json.value("identifier").toString();
    m_impl->definitionPath = json.value("definitionPath").toString();
    m_impl->uniqueIndex = json.value("uniqueIndex").toInt(0);
    if(json.contains("laserSetup"))
    {
        const QJsonObject setupObj = json.value("laserSetup").toObject();
        LaserSetup setup;
        setup.masterIntensity = setupObj.value("masterIntensity").toDouble(1.0);
        setup.testFrame = setupObj.value("testFrame").toInt(0);
        setup.sizeX = setupObj.value("sizeX").toDouble(1.0);
        setup.sizeY = setupObj.value("sizeY").toDouble(1.0);
        setup.positionX = setupObj.value("positionX").toDouble(0.0);
        setup.positionY = setupObj.value("positionY").toDouble(0.0);
        setup.rotation = setupObj.value("rotation").toDouble(0.0);
        QMutexLocker lock(&m_impl->laserSetupMutex);
        m_impl->laserSetup = setup;
    }
    m_impl->laserPreviewFolder = json.value("laserPreviewFolder").toString();
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
    json.insert("modelType", m_impl->modelType);
    json.insert("beamStyle", m_impl->beamStyle);
    json.insert("panOffset", double(m_impl->panOffset));
    json.insert("tiltOffset", double(m_impl->tiltOffset));
    json.insert("panFlip", m_impl->panFlip);
    json.insert("tiltFlip", m_impl->tiltFlip);
    json.insert("panInvert", m_impl->panInvert);
    json.insert("tiltInvert", m_impl->tiltInvert);
    json.insert("identifier", m_impl->identifier);
    json.insert("selectedMode", m_impl->selectedMode);
    json.insert("definitionPath", m_impl->definitionPath);
    json.insert("uniqueIndex", m_impl->uniqueIndex);

    if(isLaser())
    {
        const LaserSetup setup = laserSetup();
        QJsonObject setupObj;
        setupObj.insert("masterIntensity", setup.masterIntensity);
        setupObj.insert("testFrame", setup.testFrame);
        setupObj.insert("sizeX", setup.sizeX);
        setupObj.insert("sizeY", setup.sizeY);
        setupObj.insert("positionX", setup.positionX);
        setupObj.insert("positionY", setup.positionY);
        setupObj.insert("rotation", setup.rotation);
        json.insert("laserSetup", setupObj);
        json.insert("laserPreviewFolder", m_impl->laserPreviewFolder);
    }
}

} // namespace photon

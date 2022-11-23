#include <QDebug>
#include <QVariant>
#include <QHash>
#include <QJsonDocument>
#include <QFile>
#include <QJsonObject>
#include <QColor>
#include <QPalette>
#include <QPainter>
#include <QSettings>
#include "settings.h"
#include "photoncore.h"
#include "resourcemanager.h"

namespace photon {

struct SettingData
{
    enum SettingType{
        SettingUnknown,
        SettingColor,
        SettingDouble,
        SettingInteger,
        SettingBool,
        SettingString
    };

    SettingData(){}
    SettingData(const QVariant &var, SettingType type, int index):value(var),type(type),fileIndex(index){}
    QVariant value;
    SettingType type;
    int fileIndex;
};

class Settings::Impl
{
public:

    struct PageData
    {
        PageData(const SettingsPageId &t_id, std::function<SettingsPage*()> t_pageCallback, int t_priority = 0):id(t_id),pageCallback(t_pageCallback),priority(t_priority){}
        SettingsPageId id;
        std::function<SettingsPage*()> pageCallback;
        int priority = 0;
    };


    Impl();
    ~Impl();

    bool loadFromFile(const QString &path, bool allowOverwrite = false);

    int size() const;

    bool saveToFiles();

    bool exists(const QByteArray &key);
    bool setValue(const QByteArray &key, const QVariant &value);
    QVariant getValue(const QByteArray &key, const QVariant &defaultValue = QVariant()) const;
    QString injectSettings(const QString &toParse, bool *error) const;
    void regeneratePalette();
    QPalette palette() const;
    QBrush alphaBGBrush;

    QVector<PageData> pages;

    QColor colorBackground;
    QColor colorFrame;
    QColor colorAccent;

    bool stylesheetIsDirty = true;
    double parameterScaleIncrease = 2.5;
    double parameterScaleDecrease = .25;
    QString rawStylesheet;
    QString processedStylesheet;

private:
    QPalette m_palette;
    QString m_filePath;
    QHash<QByteArray, SettingData> m_settingHash;
    QVector<QString> m_filePaths;
};

Settings::Impl::Impl()
{
    int brushWidth = 10;
    int halfWidth = 5;
    QPixmap alphaPix = QPixmap{brushWidth,brushWidth};
    alphaPix.fill(Qt::white);
    QPainter pPainter(&alphaPix);
    pPainter.setPen(Qt::NoPen);
    pPainter.setBrush(QColor(200,200,200));
    pPainter.drawRect(0,0,halfWidth,halfWidth);
    pPainter.drawRect(halfWidth,halfWidth,halfWidth,halfWidth);
    alphaBGBrush = QBrush{alphaPix};

}

Settings::Impl::~Impl()
{

}

SettingData jsonToSetting(const QJsonObject obj, int index, bool *ok = nullptr)
{
    *ok = true;
    const QString typeString = obj.value("type").toString().toLower();
    if(typeString =="string")
        return SettingData{QVariant(obj.value("value").toVariant()),SettingData::SettingString,index};
    else if(typeString =="color")
    {
        if(obj.value("value").isString() && QColor::isValidColor(obj.value("value").toString()))
        {
            return SettingData{QColor(obj.value("value").toString()),SettingData::SettingColor,index};
        }
        *ok = false;
        qWarning() << "Cannot convert: '" << obj.value("value") << "' to color";
    }
    else if(typeString =="integer" || typeString =="int")
    {
        if(obj.value("value").isDouble())
            return SettingData{obj.value("value").toVariant(),SettingData::SettingInteger,index};
        *ok = false;
        qWarning() << "Cannot convert: '" << obj.value("value") << "' to double";
    }
    else if(typeString =="double" || typeString =="float")
    {
        if(obj.value("value").isDouble())
            return SettingData{obj.value("value").toVariant(),SettingData::SettingDouble,index};
        *ok = false;
        qWarning() << "Cannot convert: '" << obj.value("value") << "' to double";
    }
    else if(typeString =="bool" || typeString =="boolean")
    {
        if(obj.value("value").isBool())
            return SettingData{obj.value("value").toVariant(),SettingData::SettingBool,index};
        *ok = false;
        qWarning() << "Cannot convert: '" << obj.value("value") << "' to bool";
    }

    return SettingData{QVariant(obj.value("value").toVariant()),SettingData::SettingUnknown,index};
}

QJsonObject settingToJson(const SettingData &var)
{
    QJsonObject result;
    switch (var.type) {
        case SettingData::SettingString:
            result.insert("type","string");
            result.insert("value",var.value.toString());
        break;
        case SettingData::SettingColor:
            result.insert("type","color");
            result.insert("value",var.value.value<QColor>().name());
        break;
        case SettingData::SettingDouble:
            result.insert("type","double");
            result.insert("value",var.value.toDouble());
        break;
        case SettingData::SettingInteger:
            result.insert("type","int");
            result.insert("value",var.value.toInt());
        break;
        case SettingData::SettingBool:
            result.insert("type","bool");
            result.insert("value",var.value.toBool());
        break;
        case SettingData::SettingUnknown:
            result.insert("type","unknown");
            result.insert("value",var.value.toString());
        break;
    }
    return result;
}

bool Settings::Impl::loadFromFile(const QString &path, bool allowOverwrite)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    int fileIndex = m_filePaths.size();
    m_filePaths.append(path);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if(doc.isObject())
    {
        const QJsonObject dataObj = doc.object();
        QJsonObject::const_iterator objIt = dataObj.begin();
        while(objIt != dataObj.end()) {
            bool ok = true;
            if(!allowOverwrite && m_settingHash.contains(objIt.key().toLatin1()))
                qWarning() << "Setting '" << objIt.key() << "' cannot be overridden.";
            else
            {
                SettingData var = jsonToSetting(objIt.value().toObject(),fileIndex, &ok);
                if(ok)
                    m_settingHash.insert(objIt.key().toLatin1(),var);
                else
                    qWarning() << "Could not insert '" << objIt.key() << "'  value: " << var.value;
            }
            //qDebug() << "load setting: " << objIt.key() << "  value: " << objIt.value();
            ++objIt;
        }
        stylesheetIsDirty = true;
        regeneratePalette();
        return true;
    } else {
        qWarning() << "Json could not be parsed.";
        return false;
    }
}

QString Settings::Impl::injectSettings(const QString &toParse, bool *error) const
{
    QString result;

    int j = 0;
    int prevJ = j;
    int openBracket;
    *error = false;

    while ((j = toParse.indexOf("[[", j)) != -1) {
        result += toParse.mid(prevJ,j-prevJ);
        ++j;
        openBracket = j+1;

        j = toParse.indexOf("]]", j);
        if(j == -1)
        {
            *error = true;
            qWarning() << "error parsing stylesheet, no closing bracket after position: " << openBracket;
            return "";
        }

        result += m_settingHash.value(toParse.mid(openBracket,(j-openBracket)).toLatin1()).value.toString();
        ++j;//temp
        ++j;
        prevJ = j;

    }

    result += toParse.mid(prevJ,j-prevJ);

    if(result.indexOf("]]") != -1 || result.indexOf("[[") != -1)
    {
        *error = true;
        qWarning() << "error parsing stylesheet, mismatched brackets.";
        return "";
    }

    return result;
}

bool Settings::Impl::saveToFiles()
{
    int index = 0;
    for(QString &path : m_filePaths)
    {
        QJsonDocument doc;
        QJsonObject rootObj;

        for(auto it = m_settingHash.cbegin(); it != m_settingHash.cend(); ++it)
        {
            if(it.value().fileIndex == index)
            {
                rootObj.insert(it.key(), settingToJson(it.value()));
            }
        }
        doc.setObject(rootObj);

        QFile file(path);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Could not open file: " << path;
            return false;
        }

        file.write(doc.toJson());
        file.close();

        ++index;
    }
    return false;
}

int Settings::Impl::size() const
{
    return m_settingHash.size();
}

bool Settings::Impl::exists(const QByteArray &key)
{
    return m_settingHash.contains(key);
}

bool Settings::Impl::setValue(const QByteArray &key, const QVariant &value)
{
    QHash<QByteArray, SettingData>::iterator it = m_settingHash.find(key);

    if(it != m_settingHash.end())
    {
        if((*it).value == value)
            return false;
        else
            (*it).value = value;
    }
    else
        m_settingHash.insert(key, SettingData(value,SettingData::SettingUnknown,-1));

    stylesheetIsDirty = true;
    regeneratePalette();
    return true;
}

void Settings::Impl::regeneratePalette()
{
    colorBackground = getValue("deco.settings.ui.color.window_background").value<QColor>();
    colorFrame = getValue("deco.settings.ui.color.window_frame").value<QColor>();
    colorAccent = getValue("deco.settings.ui.color.accent").value<QColor>();

    m_palette = QPalette();
    m_palette.setBrush(QPalette::Window, colorBackground);
    m_palette.setBrush(QPalette::Base, colorBackground);
    m_palette.setBrush(QPalette::WindowText, getValue("deco.settings.ui.color.window.text").value<QColor>());
    m_palette.setBrush(QPalette::Text, getValue("deco.settings.ui.color.parameter.text").value<QColor>());
    m_palette.setBrush(QPalette::Highlight, colorAccent);

}

QPalette Settings::Impl::palette() const
{
    return m_palette;
}

QVariant Settings::Impl::getValue(const QByteArray &key, const QVariant &defaultValue) const
{
    return m_settingHash.value(key, SettingData(defaultValue,SettingData::SettingUnknown,-1)).value;
}

Settings::Settings(QObject *parent) : QObject(parent), m_impl(new Settings::Impl())
{
    //connect(photonApp,SIGNAL(applicationQuitBegan()),this,SLOT(saveToFiles()));
}

Settings::~Settings()
{

}

void Settings::init()
{
    m_impl->parameterScaleDecrease = getValue("deco.settings.parameter.scale.decrease",.25).toDouble();
    m_impl->parameterScaleIncrease = getValue("deco.settings.parameter.scale.increase",2.5).toDouble();

    QSettings set;
    set.beginGroup("app");

    set.endGroup();
}

void Settings::initPostPlugins()
{

}


QPalette Settings::palette() const
{
    return m_impl->palette();
}

QBrush Settings::alphaBackgroundBrush() const
{
    return m_impl->alphaBGBrush;
}

void Settings::registerSettingsPage(const SettingsPageId &pageId, std::function<SettingsPage*()> cb, int t_priority)
{
    for(auto it = m_impl->pages.cbegin(); it != m_impl->pages.cend(); ++it)
    {
        if(t_priority > (*it).priority)
        {
            m_impl->pages.insert(it, Impl::PageData{pageId, cb, t_priority});
            return;
        }
    }
    m_impl->pages.append(Impl::PageData{pageId, cb, t_priority});
}

QList<SettingsPageId> Settings::pageIds() const
{
    QList<SettingsPageId> results;
    for(const auto &page : m_impl->pages)
        results.append(page.id);
    return results;
}

SettingsPage *Settings::pageForId(const SettingsPageId &pageId) const
{
    for(const auto &page : m_impl->pages)
    {
        if(page.id == pageId)
            return page.pageCallback();
    }
    return nullptr;

}

bool Settings::loadFromFile(const QString &path)
{
    if( m_impl->loadFromFile(path))
    {
        emit settingsChanged();
        return true;
    }
    return false;
}

void Settings::saveToFiles()
{
    m_impl->saveToFiles();
}

bool Settings::exists(const QByteArray &key)
{
    return m_impl->exists(key);
}

void Settings::setValue(const QByteArray &key, const QVariant &value)
{
    if(m_impl->setValue(key, value))
    {
        emit valueChanged(key, value);
        //emit settingsChanged();
    }
}

QVariant Settings::getValue(const QByteArray &key, const QVariant &defaultValue) const
{
    return m_impl->getValue(key, defaultValue);
}

QColor Settings::getColor(const QByteArray &key) const
{
    return m_impl->getValue(key).value<QColor>();
}

int Settings::size() const
{
    return m_impl->size();
}

QString Settings::globalStylesheet() const
{
    if(m_impl->stylesheetIsDirty)
    {
        bool error;
        m_impl->processedStylesheet = injectSettings(m_impl->rawStylesheet, &error);
        m_impl->stylesheetIsDirty = false;
    }
    return m_impl->processedStylesheet;
}

void Settings::appendStylesheet(const QString &stylesheet)
{
    m_impl->rawStylesheet += stylesheet + "\n";
    m_impl->stylesheetIsDirty = true;
    emit settingsChanged();
}

QString Settings::injectSettings(const QString &toParse, bool *error) const
{
    return m_impl->injectSettings(toParse, error);
}

double Settings::parameterScale(Qt::KeyboardModifiers modifier)
{
    double scale = 1.0;
    if(modifier & Qt::AltModifier)
        scale *= m_impl->parameterScaleDecrease;
    else if(modifier & Qt::ControlModifier)
        scale *= m_impl->parameterScaleIncrease;
    return scale;
}

QColor Settings::backgroundColor() const
{
    return m_impl->colorBackground;
}
QColor Settings::frameColor() const
{
    return m_impl->colorFrame;
}
QColor Settings::accentColor() const
{
    return m_impl->colorAccent;
}

} // namespace deco

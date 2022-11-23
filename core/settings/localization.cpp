#include <QKeySequence>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAction>
#include "localization.h"
#include "iconfactory.h"
#include "resourcemanager.h"
#include "decocore.h"
#include "command/command.h"

namespace deco {



class Localization::Impl
{
public:

    void parseString(const QString &prefix, const QJsonObject &obj);

    QHash<TranslateId, QString> stringHash;
    QHash<TranslateId, QStringList> stringListHash;
    QVector<QString> commonWords;
    QMultiHash<QByteArray, QKeySequence> shortcutHash;
    LanguageInfo language;
    QVector<LanguageInfo> availableLanguages;
};

Localization::Localization(QObject *parent) : QObject(parent), m_impl(new Impl())
{
}

QList<QKeySequence> Localization::shortcutsForId(const QByteArray &code) const
{
    return m_impl->shortcutHash.values(code);
}

QString Localization::translateIndex(Translation index) const
{
    if(static_cast<int>(index) < m_impl->commonWords.length())
        return m_impl->commonWords[static_cast<int>(index)];
    return "";
}

QString Localization::translateString(const TranslateId &id) const
{
    return m_impl->stringHash.value(id, id);
}

QStringList Localization::translateStringList(const TranslateId &id) const
{
    return m_impl->stringListHash.value(id);
}

LanguageInfoList Localization::languages() const
{
    return m_impl->availableLanguages;
}

LanguageInfo Localization::activeLanguage() const
{
    return m_impl->language;
}

QString Localization::languageCode() const
{
    return m_impl->language.languageCode;
}

QAction *Localization::createAction(const QByteArray &id) const
{
    QAction *action = new QAction();
    localizeAction(action, id);
    return action;
}

void Localization::localizeAction(QAction *action, const QByteArray &id) const
{
    if(m_impl->stringHash.contains(id + ".title"))
        action->setText(m_impl->stringHash.value(id + ".title"));
    else
    {
        if(m_impl->stringHash.contains(id))
             action->setText(m_impl->stringHash.value(id));
        else
        {
            action->setText(id);
            //qWarning() << "could not find title for actionId: " << id;
        }
    }

    if(m_impl->stringHash.contains(id + ".tool_tip"))
        action->setToolTip(m_impl->stringHash.value(id + ".tool_tip"));


    if(decoApp->icons()->containsIcon(id))
        action->setIcon(decoApp->icons()->createIcon(id));

    action->setShortcuts(shortcutsForId(id));
    action->setShortcutContext(Qt::WidgetShortcut);
}

void Localization::localizeCommand(CommandPtr command, const QByteArray &id) const
{
    localizeAction(command->action(), id);

    /*
    auto shortcuts = shortcutsForId(id);
    if(shortcuts.length() > 0)
        command->setShortcut(shortcuts.first());
        */
}

void Localization::Impl::parseString(const QString &prefix, const QJsonObject &obj)
{
    QByteArray pref = prefix.toLatin1();
    QJsonObject::const_iterator objIt = obj.begin();
    while(objIt != obj.end()) {
        if(objIt.value().isArray())
        {
            QStringList list;
            const QJsonArray &stringsArray = objIt.value().toArray();
            for(auto str : stringsArray)
                list.append(str.toString());
            stringListHash.insert(pref + objIt.key().toLatin1(), list);
        }
        else if(objIt.value().isObject())
        {
            parseString(pref + objIt.key()+".", objIt.value().toObject());
        }
        else
        {
            if(objIt.key() == "_")
            {
                stringHash.insert(pref.left(pref.length()-1), objIt.value().toString());
            }
            else
                stringHash.insert(pref + objIt.key().toLatin1(), objIt.value().toString());
        }
        ++objIt;
    }
}

bool Localization::loadFromResource(const Resource &resource)
{
    QJsonDocument doc = resource.loadToJson();
    if(doc.isObject())
    {
        const QJsonObject &docObj = doc.object();
        LanguageInfo info;
        info.languageCode = docObj.value("language_code").toString();
        info.language = docObj.value("language").toString();
        info.languageNative = docObj.value("language_native").toString();



        const QJsonObject &stringsObj = docObj.value("strings").toObject();
        m_impl->parseString("", stringsObj);

        const QJsonArray &stringsArray = docObj.value("common").toArray();
        for(auto str : stringsArray)
            m_impl->commonWords.append(str.toString());

        //m_impl->shortcutHash.clear();
        const QJsonObject &shortcutsObj = docObj.value("shortcuts").toObject();
        QJsonObject::const_iterator objIt = shortcutsObj.begin();
        while(objIt != shortcutsObj.end()) {
            if(objIt.value().isArray())
            {
                QJsonArray shortcutArray = objIt.value().toArray();
                for(auto shortcut : shortcutArray)
                {
                    m_impl->shortcutHash.insertMulti(objIt.key().toLatin1(),QKeySequence(shortcut.toString()));
                }
            }
            else
                m_impl->shortcutHash.insert(objIt.key().toLatin1(),QKeySequence(objIt.value().toString()));
            ++objIt;
        }

        if(std::find_if(m_impl->availableLanguages.cbegin(),m_impl->availableLanguages.cend(),[info](const LanguageInfo &cur){return cur.languageCode == info.languageCode;}) == m_impl->availableLanguages.cend())
        {
            m_impl->availableLanguages.append(info);
            m_impl->language = info;
        }

        emit languageChanged();
        return true;
    } else {
        qDebug() << resource.path() << " is malformed JSON";
    }
    return false;
}

} // namespace deco

#ifndef INTERNATIONALIZATION_H
#define INTERNATIONALIZATION_H

#include <QObject>
#include "deco-core_global.h"

class QAction;

namespace deco {

struct DECOCORESHARED_EXPORT LanguageInfo
{
    QString language;
    QString languageNative;
    QString languageCode;
    QString path;
};

using LanguageInfoList = QVector<LanguageInfo>;

class DECOCORESHARED_EXPORT Localization : public QObject
{
    Q_OBJECT
public:



    explicit Localization(QObject *parent = nullptr);

    LanguageInfoList languages() const;
    LanguageInfo activeLanguage() const;
    void loadLanguage(QByteArray languageCode);
    QString languageCode() const;
    QString translateIndex(Translation index) const;
    QString translateString(const TranslateId &id) const;
    QStringList translateStringList(const TranslateId &id) const;
    void localizeAction(QAction *action, const QByteArray &id) const;
    void localizeCommand(CommandPtr command, const QByteArray &id) const;
    QAction *createAction(const QByteArray &id) const;
    QList<QKeySequence> shortcutsForId(const QByteArray &code) const;
    bool loadFromResource(const Resource &resource);
signals:

    void languageChanged();

public slots:

private:
    class Impl;
    Impl *m_impl;
};

} // namespace deco

#endif // INTERNATIONALIZATION_H

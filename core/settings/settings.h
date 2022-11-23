#ifndef SETTINGS_H
#define SETTINGS_H

#include <QScopedPointer>

#include <QString>
#include <QJsonDocument>
#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Settings : public QObject
{
    Q_OBJECT
public:

    Settings(QObject *parent = nullptr);
    ~Settings();

    bool loadFromFile(const QString &path);

    void init();
    void initPostPlugins();
    bool exists(const QByteArray &key);
    int size() const;
    void setValue(const QByteArray &key, const QVariant &value);
    QVariant getValue(const QByteArray &key, const QVariant &defaultValue = QVariant()) const;
    QColor getColor(const QByteArray &key) const;
    QString injectSettings(const QString &toParse, bool *error) const;
    QPalette palette() const;

    void registerSettingsPage(const SettingsPageId &pageId, std::function<SettingsPage*()> cb, int t_priority = 0);
    QList<SettingsPageId> pageIds() const;
    SettingsPage *pageForId(const SettingsPageId &pageId) const;

    QString globalStylesheet() const;
    void appendStylesheet(const QString &stylesheet);

    double parameterScale(Qt::KeyboardModifiers modifier);
    QColor backgroundColor() const;
    QColor frameColor() const;
    QColor accentColor() const;
    QBrush alphaBackgroundBrush() const;

private slots:
    void saveToFiles();

signals:
    void settingsLoaded();
    void settingsChanged();
    void valueChanged(const QByteArray &key, const QVariant &value);

private:

    class Impl;
    QScopedPointer<Impl> const m_impl;
};


} // namespace deco

#endif // SETTINGS_H

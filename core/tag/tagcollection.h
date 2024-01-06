#ifndef PHOTON_TAGCOLLECTION_H
#define PHOTON_TAGCOLLECTION_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT TagCollection : public QObject
{
    Q_OBJECT
public:

    explicit TagCollection(QObject *parent = nullptr);
    ~TagCollection();

    void clear();
    int tagCount() const;
    QString tagAtIndex(uint) const;
    const QStringList &tags() const;

signals:
    void tagWillBeAdded(QString, int);
    void tagWasAdded(QString, int);
    void tagWillBeRemoved(QString, int);
    void tagWasRemoved(QString, int);
    void tagsWillBeReplaced();
    void tagsWereReplaced();

public slots:
    void addTag(const QString&);
    void removeTag(const QString&);
    void replaceTags(const QStringList &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TAGCOLLECTION_H

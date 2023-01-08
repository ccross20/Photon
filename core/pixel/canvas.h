#ifndef PHOTON_CANVAS_H
#define PHOTON_CANVAS_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Canvas : public QObject
{
    Q_OBJECT
public:
    explicit Canvas(QObject *parent = nullptr);
    Canvas(const QString &name,  const QSize &size, QObject *parent = nullptr);
    ~Canvas();

    QSize size() const;
    void setSize(const QSize &);

    int width() const;
    int height() const;

    QString name() const;
    void setName(const QString &);

    void paint(const QImage &);


    void restore(Project &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    void metadataUpdated();
    void didPaint(const QImage &);

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_CANVAS_H

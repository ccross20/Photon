#ifndef PHOTON_COLORRESOURCE_H
#define PHOTON_COLORRESOURCE_H

#include <QColor>
#include "photon-global.h"
#include "project/projectresource.h"

namespace photon {

// A single named, saved colour - shown in the project panel's "Colors"
// section and reachable from the global colour picker's saved-colours strip
// (see ColorSelectorWidget::setSavedColorsProvider()) and from
// SavedColorNode in the node graph.
class PHOTONCORE_EXPORT ColorResource : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    ColorResource();
    ~ColorResource();

    QByteArray uniqueId() const;

    QString name() const;
    void setName(const QString &);

    QColor color() const;
    void setColor(const QColor &);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "color";}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override;

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORRESOURCE_H

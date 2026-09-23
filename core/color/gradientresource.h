#ifndef PHOTON_GRADIENTRESOURCE_H
#define PHOTON_GRADIENTRESOURCE_H

#include "photon-global.h"
#include "project/projectresource.h"
#include "util/gradient.h"

namespace photon {

// A single named, saved gradient - shown in the project panel's "Gradients"
// section and reachable from the node graph via SavedGradientNode.
class PHOTONCORE_EXPORT GradientResource : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    GradientResource();
    ~GradientResource();

    QByteArray uniqueId() const;

    QString name() const;
    void setName(const QString &);

    Gradient gradient() const;
    void setGradient(const Gradient &);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "gradient";}
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

#endif // PHOTON_GRADIENTRESOURCE_H

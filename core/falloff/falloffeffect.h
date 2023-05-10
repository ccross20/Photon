#ifndef PHOTON_FALLOFFEFFECT_H
#define PHOTON_FALLOFFEFFECT_H

#include <QWidget>
#include "photon-global.h"

namespace photon {


struct FalloffEffectInformation
{

    FalloffEffectInformation(){}
    FalloffEffectInformation(std::function<FalloffEffect*()> _callback):callback(_callback){}

    QByteArray effectId;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    std::function<FalloffEffect*()> callback;

    CategoryList categories;
};

class PHOTONCORE_EXPORT FalloffEffect
{
public:
    FalloffEffect(const QByteArray &id = QByteArray{});
    virtual ~FalloffEffect();

    QByteArray id() const;
    QByteArray uniqueId() const;
    void setId(const QByteArray &);
    void setName(const QString &);
    QString name() const;

    Clip *clip() const;
    int index() const;
    virtual double falloff(Fixture *) const = 0;
    virtual QWidget *createEditor() {return new QWidget();}
    void updated();

    FalloffEffect *previousEffect() const;
    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

private:
    friend class Clip;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FALLOFFEFFECT_H

#ifndef PHOTON_MASKEFFECT_H
#define PHOTON_MASKEFFECT_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

struct MaskEffectInformation
{

    MaskEffectInformation(){}
    MaskEffectInformation(std::function<MaskEffect*()> _callback):callback(_callback){}

    QByteArray effectId;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    std::function<MaskEffect*()> callback;

    CategoryList categories;
};

class PHOTONCORE_EXPORT MaskEffect
{
public:
    MaskEffect(const QByteArray &id = QByteArray{});
    virtual ~MaskEffect();

    QByteArray id() const;
    QByteArray uniqueId() const;
    void setId(const QByteArray &);
    void setName(const QString &);
    QString name() const;
    Clip *clip() const;

    int index() const;
    virtual QVector<Fixture*> process(const QVector<Fixture*> fixtures) const;
    virtual QWidget *createEditor() {return new QWidget();}
    void updated();

    MaskEffect *previousEffect() const;
    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;


private:
    friend Clip;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MASKEFFECT_H

#ifndef PHOTON_FALLOFFMAP2D_H
#define PHOTON_FALLOFFMAP2D_H

#include "fixturefalloffmap.h"

namespace photon {

class PHOTONCORE_EXPORT FalloffMap2D : public SceneFalloffMap
{
public:

    struct Falloff2DData{
        QByteArray sceneObjectId;
        QPointF position;
    };

    FalloffMap2D();
    FalloffMap2D(const FalloffMap2D &other);
    ~FalloffMap2D();

    FalloffMap2D &operator=(const FalloffMap2D &other);

    void addSceneObject(SceneObject *, const QPointF &);
    void removeSceneObject(SceneObject *);
    void setSceneObjectPosition(SceneObject *, const QPointF &);
    QPointF sceneObjectPosition(SceneObject *) const;
    double falloff(SceneObject *) const override;

    QVector<Falloff2DData> sceneObjectData() const;
    void setSceneObjectData(const QVector<Falloff2DData> &);

    QPointF startPosition() const;
    QPointF endPosition() const;
    void setStartPosition(const QPointF &);
    void setEndPosition(const QPointF &);


    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FALLOFFMAP2D_H

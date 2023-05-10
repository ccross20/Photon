#ifndef PHOTON_TRUSS_H
#define PHOTON_TRUSS_H

#include <QWidget>
#include "photon-global.h"
#include "scene/sceneobject.h"

namespace photon {

class Truss;

class TrussEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrussEditorWidget(Truss *,QWidget *parent = nullptr);
    ~TrussEditorWidget();

    void setFixtures(QVector<Fixture*> fixtures);

signals:

private slots:
    void setName(const QString &name);
    void setBeams(int);
    void setSegmentLength(float);
    void setRadius(float);
    void setOffset(float);
    void setLength(float);
    void setAngle(float);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);


private:
    class Impl;
    Impl *m_impl;

};

class PHOTONCORE_EXPORT Truss : public SceneObject
{
    Q_OBJECT
public:
    const static QByteArray TrussMime;

    Truss();
    ~Truss();

    void setBeams(uint);
    void setSegmentLength(float);
    void setRadius(float);
    void setOffset(float);
    void setLength(float);
    void setAngle(float t_degrees);

    uint beams() const;
    float radius() const;
    float offset() const;
    float segmentLength() const;
    float length() const;
    float angle() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TRUSS_H

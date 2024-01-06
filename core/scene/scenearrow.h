#ifndef PHOTON_SCENEARROW_H
#define PHOTON_SCENEARROW_H

#include <QWidget>
#include "photon-global.h"
#include "scene/sceneobject.h"

namespace photon {

class SceneArrow;



class SceneArrowEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneArrowEditorWidget(SceneArrow *,QWidget *parent = nullptr);
    ~SceneArrowEditorWidget();

signals:

private slots:
    void setName(const QString &name);
    void setSize(float);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);


private:
    class Impl;
    Impl *m_impl;

};



class PHOTONCORE_EXPORT SceneArrow : public SceneObject
{
public:
    SceneArrow();
    ~SceneArrow();

    void setSize(float);
    float size() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEARROW_H

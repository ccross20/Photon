#ifndef PHOTON_SCENEDIRECTION_H
#define PHOTON_SCENEDIRECTION_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class SceneDirection;

class SceneDirectionEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneDirectionEditorWidget(SceneDirection *, QWidget *parent = nullptr);
    ~SceneDirectionEditorWidget();

private slots:
    void setName(const QString &name);
    void setSize(float);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);

private:
    class Impl;
    Impl *m_impl;
};

// A vector in 3D space - a facing/aiming direction rather than a general
// pointer annotation (see SceneArrow). Same viewport shape (shaft + arrowhead
// along local +Y), kept as its own type so it can be organized/labeled
// separately from decorative Arrows. Color and visibility mode come from
// SceneHelperObject.
class PHOTONCORE_EXPORT SceneDirection : public SceneHelperObject
{
    Q_OBJECT
public:
    SceneDirection();
    ~SceneDirection();

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

#endif // PHOTON_SCENEDIRECTION_H

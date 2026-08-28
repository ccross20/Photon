#ifndef PHOTON_SCENEARROW_H
#define PHOTON_SCENEARROW_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

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



// A viewport-only pointer/annotation shape, drawn as a shaft + arrowhead
// along local +Y. Color and visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT SceneArrow : public SceneHelperObject
{
    Q_OBJECT
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

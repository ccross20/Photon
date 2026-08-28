#ifndef PHOTON_SCENEPOINTMARKER_H
#define PHOTON_SCENEPOINTMARKER_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class ScenePointMarker;

class ScenePointMarkerEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScenePointMarkerEditorWidget(ScenePointMarker *, QWidget *parent = nullptr);
    ~ScenePointMarkerEditorWidget();

private slots:
    void setName(const QString &name);
    void setShape(int index);
    void setSize(double);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);

private:
    class Impl;
    Impl *m_impl;
};

// A "null object"-style point of interest - marks a location (e.g. where a
// cake table or podium will be) with a small 2D glyph in the local XY plane.
// Color and visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT ScenePointMarker : public SceneHelperObject
{
    Q_OBJECT
public:
    enum Shape { ShapeStar, ShapeCross, ShapeCircle, ShapeSquare, ShapeDiamond };

    ScenePointMarker();
    ~ScenePointMarker();

    void setShape(Shape);
    Shape shape() const;

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

#endif // PHOTON_SCENEPOINTMARKER_H

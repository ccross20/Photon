#ifndef PHOTON_SCENEBOX_H
#define PHOTON_SCENEBOX_H

#include <QWidget>
#include <QColor>
#include <QVector3D>
#include "photon-global.h"
#include "scene/sceneobject.h"

namespace photon {

class SceneBox;

class SceneBoxEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneBoxEditorWidget(SceneBox *, QWidget *parent = nullptr);
    ~SceneBoxEditorWidget();

private slots:
    void setName(const QString &name);
    void setSize(const QVector3D &);
    void chooseColor();
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);
    void refreshTransform();
    void refreshSize();

private:
    class Impl;
    Impl *m_impl;
};

// A solid box (riser, set piece, booth, pillar) that fixtures light like a
// wall or floor. Centered at the object origin with full extents = size(), in
// metres; orient/position it with the transform gizmo and resize it with the
// scale handles.
class PHOTONCORE_EXPORT SceneBox : public SceneObject
{
    Q_OBJECT
public:
    // Each side is kept at least this long so the box never collapses.
    static constexpr float MinimumSide = 0.01f;

    SceneBox();
    ~SceneBox();

    void setSize(const QVector3D &);
    void setColor(const QColor &);

    QVector3D size() const;
    QColor color() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEBOX_H

#ifndef PHOTON_TREEMASKEFFECT_H
#define PHOTON_TREEMASKEFFECT_H
#include "fixture/maskeffect.h"
#include "gui/sceneobjectselector.h"

namespace photon {

class TreeMaskEffect;

class SelectorWatcher : public QObject
{
public:
    SelectorWatcher(TreeMaskEffect *, SceneObjectSelector *);

public slots:
    void selectionChanged();

private:
    TreeMaskEffect *m_effect;
    SceneObjectSelector *m_selector;

};

class TreeMaskEffect : public MaskEffect
{
public:
    TreeMaskEffect();

    QVector<Fixture*> process(const QVector<Fixture*> fixtures) const override;
    QWidget *createEditor() override;
    void setSelection(const QVector<SceneObject *> &);

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static MaskEffectInformation info();

private:
    QVector<QByteArray> m_selectedIds;
};

} // namespace photon

#endif // PHOTON_TREEMASKEFFECT_H

#ifndef PHOTON_TAGMASKEFFECT_H
#define PHOTON_TAGMASKEFFECT_H
#include <QCheckBox>
#include "fixture/maskeffect.h"


namespace photon {

class TagMaskEffect;
class TagEditorWidget;

class TagMaskEffectEditor :public QWidget
{
public:
    TagMaskEffectEditor(TagMaskEffect *);

private slots:
    void relatedToggle(bool);
    void allToggle(bool);

private:
    TagEditorWidget *m_tagEditor;
    QCheckBox *m_allCheck;
    QCheckBox *m_relatedCheck;
    TagMaskEffect *m_effect;
};

class TagMaskEffect : public MaskEffect
{
public:
    TagMaskEffect();

    QVector<Fixture*> process(const QVector<Fixture*> fixtures) const override;
    QWidget *createEditor() override;
    void setTags(const QStringList &);
    void setRequireAll(bool);
    void setCheckRelated(bool);
    QStringList tags() const{return m_tags;}
    bool requireAll() const{return m_requireAll;}
    bool checkRelated() const{return m_checkRelated;}

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static MaskEffectInformation info();

private:
    QStringList m_tags;
    bool m_checkRelated;
    bool m_requireAll;
};

} // namespace photon

#endif // PHOTON_TAGMASKEFFECT_H

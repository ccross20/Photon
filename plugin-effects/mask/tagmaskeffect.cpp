#include <QFormLayout>
#include "tagmaskeffect.h"
#include "util/utils.h"
#include "fixture/fixture.h"

namespace photon {


TagMaskEffectEditor::TagMaskEffectEditor(TagMaskEffect *t_effect):QWidget(),m_effect(t_effect)
{
    m_tagEdit = new QLineEdit;
    m_tagEdit->setText(m_effect->tags().join(" "));

    m_relatedCheck = new QCheckBox;
    m_relatedCheck->setChecked(m_effect->checkRelated());

    m_allCheck = new QCheckBox;
    m_allCheck->setChecked(m_effect->requireAll());

    QFormLayout *form = new QFormLayout;
    form->addRow("Tags", m_tagEdit);
    form->addRow("Related", m_relatedCheck);
    form->addRow("Require All", m_allCheck);

    setLayout(form);

    connect(m_tagEdit, &QLineEdit::textChanged, this, &TagMaskEffectEditor::tagsUpdated);
    connect(m_relatedCheck, &QCheckBox::toggled, this, &TagMaskEffectEditor::relatedToggle);
    connect(m_allCheck, &QCheckBox::toggled, this, &TagMaskEffectEditor::allToggle);
}

void TagMaskEffectEditor::tagsUpdated(QString t_text)
{
    m_effect->setTags(t_text.toLower().split(" "));
}

void TagMaskEffectEditor::relatedToggle(bool t_value)
{
    m_effect->setCheckRelated(t_value);
}

void TagMaskEffectEditor::allToggle(bool t_value)
{
    m_effect->setRequireAll(t_value);
}

TagMaskEffect::TagMaskEffect() : MaskEffect("photon.core.tag-mask-effect")
{

}

bool stringListContains(const QStringList &t_stack, const QStringList &t_needles, bool t_all = false)
{
    int foundCount = 0;
    for(const QString &t_needle : t_needles)
    {
        if(t_stack.contains(t_needle))
        {
            foundCount++;
            if(!t_all || foundCount == t_needles.length())
                return true;
        }
    }

    return false;
}

QVector<Fixture*> TagMaskEffect::process(const QVector<Fixture*> fixtures) const
{
    QVector<Fixture*> results;

    if(m_checkRelated)
    {
        for(auto fixture : fixtures)
        {
            SceneObject *currentObj = fixture;

            do
            {
                if(stringListContains(currentObj->tags(), m_tags, m_requireAll))
                {
                    results << fixture;
                    break;
                }


                currentObj = fixture->parentSceneObject();
            }while(currentObj);
        }
    }
    else
    {
        for(auto fixture : fixtures)
        {
            if(stringListContains(fixture->tags(), m_tags, m_requireAll))
                results << fixture;
        }
    }

    return results;
}

QWidget *TagMaskEffect::createEditor()
{
    return new TagMaskEffectEditor(this);
}

void TagMaskEffect::setTags(const QStringList &t_tags)
{
    m_tags = t_tags;
    updated();
}

void TagMaskEffect::setRequireAll(bool t_value)
{
    m_requireAll = t_value;
    updated();
}

void TagMaskEffect::setCheckRelated(bool t_value)
{
    m_checkRelated = t_value;
    updated();
}

void TagMaskEffect::readFromJson(const QJsonObject &t_json)
{
    MaskEffect::readFromJson(t_json);

    m_tags = jsonToStringList(t_json.value("tags").toArray());
    m_checkRelated = t_json.value("related").toBool();
    m_requireAll = t_json.value("all").toBool();

}

void TagMaskEffect::writeToJson(QJsonObject &t_json) const
{
    MaskEffect::writeToJson(t_json);

    t_json.insert("tags",stringListToJson(m_tags));
    t_json.insert("related", m_checkRelated);
    t_json.insert("all", m_requireAll);

}



MaskEffectInformation TagMaskEffect::info()
{
    MaskEffectInformation toReturn([](){return new TagMaskEffect;});
    toReturn.name = "Tag Mask";
    toReturn.effectId = "photon.core.tag-mask-effect";

    return toReturn;
}


} // namespace photon

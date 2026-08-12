#include "tagsparameter.h"
#include "view/nodeeditor.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

const QByteArray TagsParameter::ParameterId = "tags";

TagsParameter::TagsParameter() : Parameter()
{
}

TagsParameter::TagsParameter(const QByteArray &t_id, const QString &t_name, const QString &t_default, int connectionOptions)
    : Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{
}

QWidget *TagsParameter::createWidget(keira::NodeEditor *) const
{
    // Unlike a connectable parameter, isReadOnly() here just reflects that
    // this field doesn't accept a graph connection (connectionOptions == 0
    // in every current use, e.g. SelectFixturesNode's Tags) - it's still
    // meant to be edited directly, the same way StringParameter ignores
    // isReadOnly() for its own directly-typed fields.

    // Commits directly via setValue() from the widget's own set-callback,
    // the same way FixtureListParameter's selector commits (see
    // fixturelistparameter.cpp) - TagEditorWidget has no single "value
    // changed" signal to route through the usual
    // item->widgetUpdated()/updateValue() round trip that StringParameter
    // and ColorParameter use.
    TagsParameter *param = const_cast<TagsParameter*>(this);

    auto *widget = new TagEditorWidget(
        [param](){ return param->value().toString().split(' ', Qt::SkipEmptyParts); },
        [param](const QStringList &tags){ param->setValue(tags.join(' ')); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });

    return widget;
}

void TagsParameter::updateWidget(QWidget *t_widget) const
{
    // Pushes an externally-changed value (undo, a graph reload, ...) back
    // into the chip row.
    static_cast<TagEditorWidget*>(t_widget)->refresh();
}

QVariant TagsParameter::updateValue(QWidget *) const
{
    // Not part of this parameter's actual commit path (see createWidget()) -
    // kept for contract completeness, since Node/NodeEditor call it generically.
    return value();
}

} // namespace photon

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSignalBlocker>
#include <QSet>
#include "numberscrubfield.h"
#include "gizmopropertyeditor.h"
#include "propertywidgets.h"
#include "surface/surfacegizmo.h"

namespace photon {

namespace {

// Category order, matching what the QML inspector showed. Anything with an
// unrecognised category falls into the trailing "Properties" group.
struct CategorySpec { const char *key; const char *title; };
const CategorySpec kCategories[] = {
    { "identity", "Identity" },
    { "layout",   "Layout" },
    { "style",    "Style" },
};

QString categoryOf(GizmoProperty *prop)
{
    return prop->metadata("category", "general").toString();
}

} // namespace

GizmoPropertyEditor::GizmoPropertyEditor(SurfaceGizmo *t_gizmo, QWidget *t_parent)
    : QWidget(t_parent), m_gizmo(t_gizmo)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_form = new PropertyForm;
    layout->addWidget(m_form);

    if (m_gizmo) {
        connect(m_gizmo, &SurfaceGizmo::propertyChanged,
                this, &GizmoPropertyEditor::propertyChangedExternally);
    }

    rebuild();
}

void GizmoPropertyEditor::rebuild()
{
    m_form->clear();
    m_setters.clear();

    if (!m_gizmo) {
        m_form->addStretch();
        return;
    }

    const QVector<GizmoProperty *> props = m_gizmo->properties();

    // Known categories first, in order, each only if it has members.
    QSet<QString> emitted;
    for (const CategorySpec &spec : kCategories) {
        const QString key = QString::fromUtf8(spec.key);
        QVector<GizmoProperty *> group;
        for (GizmoProperty *p : props)
            if (categoryOf(p) == key)
                group.append(p);
        if (group.isEmpty())
            continue;
        emitted.insert(key);
        m_form->addSection(QString::fromUtf8(spec.title));
        for (GizmoProperty *p : group)
            addProperty(m_form, p);
    }

    // Everything else.
    QVector<GizmoProperty *> rest;
    for (GizmoProperty *p : props) {
        const QString c = categoryOf(p);
        bool known = false;
        for (const CategorySpec &spec : kCategories)
            known = known || c == QString::fromUtf8(spec.key);
        if (!known)
            rest.append(p);
    }
    if (!rest.isEmpty()) {
        m_form->addSection(QStringLiteral("Properties"));
        for (GizmoProperty *p : rest)
            addProperty(m_form, p);
    }

    m_form->addStretch();
}

void GizmoPropertyEditor::addProperty(PropertyForm *t_form, GizmoProperty *t_prop)
{
    const QByteArray id = t_prop->id();
    const QVariantHash meta = t_prop->metadata();
    SurfaceGizmo *gizmo = m_gizmo;

    // Every write goes through here so the echo guard is applied in one place.
    auto commit = [this, gizmo, id](const QVariant &value) {
        if (!gizmo)
            return;
        m_applying = true;
        gizmo->setPropertyValue(id, value);
        m_applying = false;
    };

    QWidget *editor = nullptr;

    switch (t_prop->type()) {
    case GizmoProperty::Number: {
        editor = PropertyWidgets::createNumber(t_prop->value().toDouble(), meta,
                                               [commit](double v) { commit(v); });
        m_setters.insert(id, [editor](const QVariant &v) {
            if (auto *f = qobject_cast<NumberScrubField *>(editor))
                f->setValue(v.toDouble());
        });
        break;
    }
    case GizmoProperty::Boolean: {
        editor = PropertyWidgets::createBoolean(t_prop->value().toBool(), meta,
                                                [commit](bool v) { commit(v); });
        m_setters.insert(id, [editor](const QVariant &v) {
            if (auto *b = qobject_cast<QPushButton *>(editor)) {
                QSignalBlocker block(b);
                b->setChecked(v.toBool());
                b->setText(v.toBool() ? "On" : "Off");
            }
        });
        break;
    }
    case GizmoProperty::Text: {
        editor = PropertyWidgets::createText(t_prop->value().toString(), meta,
                                             [commit](const QString &v) { commit(v); });
        m_setters.insert(id, [editor](const QVariant &v) {
            if (auto *e = qobject_cast<QLineEdit *>(editor)) {
                if (!e->hasFocus())
                    e->setText(v.toString());
            }
        });
        break;
    }
    case GizmoProperty::Color: {
        editor = PropertyWidgets::createColor(t_prop->value().value<QColor>(), meta,
                                              [commit](const QColor &v) { commit(v); });
        break;
    }
    case GizmoProperty::Point: {
        // The QML inspector had no real point editor and fell back to a single
        // number field; two linked fields is what this type always meant.
        editor = PropertyWidgets::createPoint(t_prop->value().toPointF(), meta,
                                              [commit](const QPointF &v) { commit(v); });
        break;
    }
    case GizmoProperty::Options: {
        // Options store the chosen *string*, not its index (see the QML
        // inspector's onActivated), so map both ways here.
        const QStringList options = meta.value("options").toStringList();
        const QString current = t_prop->value().toString();
        editor = PropertyWidgets::createOptions(
            options, options.indexOf(current), meta,
            [commit, options](int index) {
                if (index >= 0 && index < options.size())
                    commit(options.at(index));
            });
        m_setters.insert(id, [editor, options](const QVariant &v) {
            if (auto *c = qobject_cast<QComboBox *>(editor)) {
                QSignalBlocker block(c);
                c->setCurrentIndex(options.indexOf(v.toString()));
            }
        });
        break;
    }
    }

    if (editor)
        t_form->addRow(t_prop->name(), editor);
}

void GizmoPropertyEditor::propertyChangedExternally(const QByteArray &t_id)
{
    if (m_applying || !m_gizmo)
        return;

    auto it = m_setters.constFind(t_id);
    if (it != m_setters.constEnd())
        (*it)(m_gizmo->propertyValue(t_id));
}

} // namespace photon

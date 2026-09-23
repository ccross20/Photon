#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QColorDialog>
#include "propertywidgets.h"
#include "numberscrubfield.h"
#include "propertycombobox.h"

namespace photon {

// Inset between a property page's rows and whatever frames it.
static constexpr int kFormMargin = 8;

namespace PropertyWidgets {

const char *MetaMinimum     = "minimum";
const char *MetaMaximum     = "maximum";
const char *MetaSoftMinimum = "softMinimum";
const char *MetaSoftMaximum = "softMaximum";
const char *MetaPrecision   = "precision";
const char *MetaReadOnly    = "readOnly";

namespace {

// Shared row height, so a scrub field, a combo and a colour swatch all line up.
constexpr int kEditorHeight = 30;

bool isReadOnly(const QVariantHash &meta)
{
    return meta.value(MetaReadOnly, false).toBool();
}

// The read-only stand-in used by every factory: a flat label rather than a
// disabled editor, matching what the Parameter widgets already did.
QWidget *readOnlyLabel(const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setMaximumHeight(kEditorHeight);
    label->setStyleSheet("background:transparent;");
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    return label;
}

NumberScrubField *makeScrubField(double value, const QVariantHash &meta, bool integer)
{
    NumberScrubField *field = new NumberScrubField;
    field->setMaximumHeight(kEditorHeight);
    field->setMinimumWidth(50);
    field->setIsInteger(integer);
    if (!integer)
        field->setDecimals(meta.value(MetaPrecision, 3).toInt());
    if (meta.contains(MetaMinimum) || meta.contains(MetaMaximum)) {
        field->setRange(meta.value(MetaMinimum, 0.0).toDouble(),
                        meta.value(MetaMaximum, 0.0).toDouble());
    }
    if (meta.contains(MetaSoftMinimum) || meta.contains(MetaSoftMaximum)) {
        field->setSoftRange(meta.value(MetaSoftMinimum, field->minimum()).toDouble(),
                            meta.value(MetaSoftMaximum, field->maximum()).toDouble());
    }
    field->setValue(value);
    return field;
}

} // namespace

void styleToggleButton(QPushButton *button)
{
    button->setMaximumHeight(kEditorHeight);
    button->setMinimumWidth(50);
    button->setCheckable(true);
    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    // A checkable QPushButton leans on the platform style for checked vs.
    // unchecked, which on macOS is a subtle shade-of-gray difference that's
    // easy to miss. Give the two states distinct colours instead.
    button->setStyleSheet(
        "QPushButton { background-color: #4a4a4a; border: 1px solid #2b2b2b; border-radius: 3px; color: #cfcfcf; }"
        "QPushButton:hover { border: 1px solid #777777; }"
        "QPushButton:checked { background-color: #3fa66c; border: 1px solid #2c7d4f; color: #ffffff; font-weight: bold; }"
        "QPushButton:checked:hover { background-color: #48ba79; }");
}

QWidget *createNumber(double value, const QVariantHash &meta,
                      std::function<void(double)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(QString::number(value));

    NumberScrubField *field = makeScrubField(value, meta, false);
    QObject::connect(field, &NumberScrubField::valueChanged, field,
                     [onChange](double v) { if (onChange) onChange(v); });
    return field;
}

QWidget *createInteger(int value, const QVariantHash &meta,
                       std::function<void(int)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(QString::number(value));

    NumberScrubField *field = makeScrubField(value, meta, true);
    QObject::connect(field, &NumberScrubField::valueChanged, field,
                     [onChange](double v) { if (onChange) onChange(int(v)); });
    return field;
}

QWidget *createBoolean(bool value, const QVariantHash &meta,
                       std::function<void(bool)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(value ? "On" : "Off");

    QPushButton *button = new QPushButton;
    styleToggleButton(button);
    button->setChecked(value);
    button->setText(value ? "On" : "Off");
    QObject::connect(button, &QPushButton::toggled, button, [button, onChange](bool v) {
        button->setText(v ? "On" : "Off");
        if (onChange) onChange(v);
    });
    return button;
}

QWidget *createText(const QString &value, const QVariantHash &meta,
                    std::function<void(const QString &)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(value);

    QLineEdit *edit = new QLineEdit(value);
    edit->setMaximumHeight(kEditorHeight);
    // editingFinished rather than textEdited: committing per keystroke makes
    // every character a separate undo step and re-evaluates the graph.
    QObject::connect(edit, &QLineEdit::editingFinished, edit,
                     [edit, onChange]() { if (onChange) onChange(edit->text()); });
    return edit;
}

QWidget *createColor(const QColor &value, const QVariantHash &meta,
                     std::function<void(const QColor &)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(value.name());

    // A button whose own background is the swatch, so it reads as a colour
    // rather than as a control containing one.
    QPushButton *swatch = new QPushButton;
    swatch->setMaximumHeight(kEditorHeight);
    swatch->setMinimumWidth(50);
    swatch->setCursor(Qt::PointingHandCursor);

    auto paint = [swatch](const QColor &c) {
        swatch->setStyleSheet(QString("QPushButton { background-color: %1; border: 1px solid #2b2b2b;"
                                      " border-radius: 3px; }"
                                      "QPushButton:hover { border: 1px solid #777777; }")
                                  .arg(c.name()));
        swatch->setProperty("photonColor", c);
    };
    paint(value.isValid() ? value : Qt::black);

    QObject::connect(swatch, &QPushButton::clicked, swatch, [swatch, paint, onChange]() {
        const QColor current = swatch->property("photonColor").value<QColor>();
        const QColor picked = QColorDialog::getColor(current, swatch, QStringLiteral("Select Color"),
                                                     QColorDialog::ShowAlphaChannel);
        if (!picked.isValid())
            return;   // dialog cancelled - leave the value untouched
        paint(picked);
        if (onChange) onChange(picked);
    });
    return swatch;
}

QWidget *createPoint(const QPointF &value, const QVariantHash &meta,
                     std::function<void(const QPointF &)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(QStringLiteral("%1, %2").arg(value.x()).arg(value.y()));

    QWidget *host = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    NumberScrubField *xField = makeScrubField(value.x(), meta, false);
    NumberScrubField *yField = makeScrubField(value.y(), meta, false);
    layout->addWidget(xField);
    layout->addWidget(yField);

    // Both fields report the whole point, so a change to either keeps the other's
    // current value rather than the one captured at build time.
    auto emitPoint = [xField, yField, onChange]() {
        if (onChange) onChange(QPointF(xField->value(), yField->value()));
    };
    QObject::connect(xField, &NumberScrubField::valueChanged, host, [emitPoint](double) { emitPoint(); });
    QObject::connect(yField, &NumberScrubField::valueChanged, host, [emitPoint](double) { emitPoint(); });
    return host;
}

QWidget *createOptions(const QStringList &options, int index, const QVariantHash &meta,
                       std::function<void(int)> onChange)
{
    if (isReadOnly(meta))
        return readOnlyLabel(index >= 0 && index < options.size() ? options.at(index) : QString());

    // PropertyComboBox rather than a plain QComboBox for its popup handling -
    // the app stylesheet otherwise gives it a menu-style popup that clips its
    // own last rows (see propertycombobox.h).
    QComboBox *combo = new PropertyComboBox;
    combo->setMaximumHeight(kEditorHeight);
    combo->addItems(options);
    if (index >= 0 && index < options.size())
        combo->setCurrentIndex(index);
    QObject::connect(combo, &QComboBox::currentIndexChanged, combo,
                     [onChange](int i) { if (onChange) onChange(i); });
    return combo;
}

} // namespace PropertyWidgets

// ---------------------------------------------------------------------------

PropertyForm::PropertyForm(QWidget *parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    // Breathing room inside whatever frames the form. Without it the editor
    // column runs flush into the panel's right edge, which reads as clipped.
    // Set here rather than on the panel so every property surface - node
    // parameters, gizmo properties, resource fields - insets identically.
    m_layout->setContentsMargins(kFormMargin, kFormMargin, kFormMargin, kFormMargin);
    m_layout->setSpacing(6);
}

QFormLayout *PropertyForm::ensureGrid()
{
    if (!m_grid) {
        m_grid = new QFormLayout;
        m_grid->setContentsMargins(0, 0, 0, 0);
        // Right-aligned labels read as "belonging to" the field beside them
        // rather than floating past their own column, and growing every
        // field to the column width keeps a combo box row and a line-edit
        // row lining up at the same right edge. Matches the fixture editor,
        // the property page this was standardized on.
        m_grid->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_grid->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        m_grid->setHorizontalSpacing(12);
        m_grid->setVerticalSpacing(8);

        // Keep the grid above any stretch that was already added.
        const int at = m_hasStretch ? m_layout->count() - 1 : m_layout->count();
        m_layout->insertLayout(at, m_grid);
    }
    return m_grid;
}

void PropertyForm::addSection(const QString &title)
{
    // objectName rather than an inline stylesheet: "propertySectionHeader" is
    // styled in styles.css (colour, underline, size), so every section header
    // in the app - this one and the fixture editor's - draws from the same
    // rule instead of two copies that could quietly drift apart. Uppercased
    // to match, and forced to Expanding so the underline spans the full row
    // rather than just the word.
    QLabel *header = new QLabel(title.toUpper());
    header->setObjectName("propertySectionHeader");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    const int at = m_hasStretch ? m_layout->count() - 1 : m_layout->count();
    m_layout->insertWidget(at, header);

    // Rows after a header belong to a new grid, so the header isn't squeezed
    // into the previous group's column widths.
    m_grid = nullptr;
    m_empty = false;
}

void PropertyForm::addRow(const QString &label, QWidget *editor)
{
    if (!editor)
        return;

    ensureGrid()->addRow(label, editor);
    m_empty = false;
}

void PropertyForm::addFullWidth(QWidget *widget, int stretch)
{
    if (!widget)
        return;

    const int at = m_hasStretch ? m_layout->count() - 1 : m_layout->count();
    m_layout->insertWidget(at, widget, stretch);

    // A full-width widget that claims the stretch makes the trailing spacer
    // redundant - otherwise the two split the leftover space.
    if (stretch > 0 && m_hasStretch) {
        m_layout->setStretch(m_layout->count() - 1, 0);
        m_hasStretch = false;
    }

    m_grid = nullptr;   // subsequent rows start a fresh grid below this widget
    m_empty = false;
}

void PropertyForm::addStretch()
{
    if (m_hasStretch)
        return;
    m_layout->addStretch(1);
    m_hasStretch = true;
}

void PropertyForm::clear()
{
    // Recursive: rows live in nested grid layouts, whose widgets would
    // otherwise survive as invisible children of this form.
    std::function<void(QLayout *)> purge = [&purge](QLayout *layout) {
        while (QLayoutItem *item = layout->takeAt(0)) {
            if (QWidget *w = item->widget())
                w->deleteLater();
            if (QLayout *child = item->layout()) {
                purge(child);
                delete child;
            }
            delete item;
        }
    };
    purge(m_layout);

    m_grid = nullptr;
    m_hasStretch = false;
    m_empty = true;
}

bool PropertyForm::isEmpty() const
{
    return m_empty;
}

} // namespace photon

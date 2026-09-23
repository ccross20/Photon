#ifndef PHOTONUI_PROPERTYWIDGETS_H
#define PHOTONUI_PROPERTYWIDGETS_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPointF>
#include <QVariantHash>
#include <functional>
#include "photon-ui-global.h"

class QFormLayout;
class QVBoxLayout;
class QPushButton;

namespace photon {

// The shared look of every property editor in the app.
//
// Three places show properties - node parameters, surface gizmos and project
// resources - and each used to build its own controls, so the same "a number"
// or "a colour" looked and behaved differently depending on where you edited
// it. These factories are the single source of those controls; a property
// editor picks the one matching its type and supplies a callback.
//
// This lives in photon-ui, below both keira and photon-core, so node
// parameters and surface gizmos can share one set of controls.
namespace PropertyWidgets {

// Metadata keys understood by the factories below (all optional):
//   "minimum", "maximum"          - double, hard bounds for the numeric
//                                   fields - the absolute ceiling on the
//                                   value; see NumberScrubField's class
//                                   comment for the hard-vs-soft rationale
//   "softMinimum", "softMaximum"  - double, the interactive (slider/fill)
//                                   range for the numeric fields; defaults to
//                                   mirroring minimum/maximum when omitted
//   "precision"                   - int, decimal places (number only)
//   "readOnly"                    - bool, renders a flat label instead of an editor
extern PHOTONUI_EXPORT const char *MetaMinimum;
extern PHOTONUI_EXPORT const char *MetaMaximum;
extern PHOTONUI_EXPORT const char *MetaSoftMinimum;
extern PHOTONUI_EXPORT const char *MetaSoftMaximum;
extern PHOTONUI_EXPORT const char *MetaPrecision;
extern PHOTONUI_EXPORT const char *MetaReadOnly;

PHOTONUI_EXPORT QWidget *createNumber(double value, const QVariantHash &meta,
                                   std::function<void(double)> onChange);
PHOTONUI_EXPORT QWidget *createInteger(int value, const QVariantHash &meta,
                                    std::function<void(int)> onChange);
PHOTONUI_EXPORT QWidget *createBoolean(bool value, const QVariantHash &meta,
                                    std::function<void(bool)> onChange);
PHOTONUI_EXPORT QWidget *createText(const QString &value, const QVariantHash &meta,
                                 std::function<void(const QString &)> onChange);
PHOTONUI_EXPORT QWidget *createColor(const QColor &value, const QVariantHash &meta,
                                  std::function<void(const QColor &)> onChange);
PHOTONUI_EXPORT QWidget *createPoint(const QPointF &value, const QVariantHash &meta,
                                  std::function<void(const QPointF &)> onChange);
PHOTONUI_EXPORT QWidget *createOptions(const QStringList &options, int index,
                                    const QVariantHash &meta,
                                    std::function<void(int)> onChange);

// Applies the shared on/off styling to a checkable button. Exposed so
// BooleanParameter can render identically without duplicating the stylesheet.
PHOTONUI_EXPORT void styleToggleButton(QPushButton *button);

} // namespace PropertyWidgets

// A property page's layout: right-aligned "Label   [editor]" rows, grouped
// under uppercase section headers, with an escape hatch for editors that own
// the full width. This is the app's one property-page layout - node
// parameters, gizmo properties and resource fields (fixtures, routines, ...)
// are all built on it, so a Number field or a section break looks and lines
// up the same regardless of what's being edited.
//
// The look (right-aligned labels, fields grown to a common column, uppercase
// underlined section headers) was standardized on the fixture editor's, the
// first property page in the app and still its most elaborate one.
class PHOTONUI_EXPORT PropertyForm : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyForm(QWidget *parent = nullptr);

    // Uppercase, underlined header that starts a new group of rows (styled by
    // the "propertySectionHeader" object name - see styles.css).
    void addSection(const QString &title);
    // One labelled row. Takes ownership of `editor`.
    void addRow(const QString &label, QWidget *editor);
    // An editor spanning the full width with no label - custom widgets, lists,
    // anything that manages its own layout. A non-zero stretch lets it absorb
    // the page's leftover height instead of the trailing spacer.
    void addFullWidth(QWidget *widget, int stretch = 0);
    // Absorbs leftover vertical space so rows stay pinned to the top. Safe to
    // call more than once; only the first has an effect.
    void addStretch();

    // Removes every row, section and widget.
    void clear();

    bool isEmpty() const;

private:
    QFormLayout *ensureGrid();

    QVBoxLayout *m_layout = nullptr;
    QFormLayout *m_grid = nullptr;   // the form rows are currently going into
    bool m_hasStretch = false;
    bool m_empty = true;
};

} // namespace photon

#endif // PHOTONUI_PROPERTYWIDGETS_H

#ifndef SEGMENTEDGIZMO_H
#define SEGMENTEDGIZMO_H
#include <QStringList>
#include "surfacegizmo.h"

namespace photon {

// A segmented control — a row (or column) of mutually-exclusive options, like
// a set of radio buttons. Options (segment labels) and Values (what the
// "value" output emits per segment) are each stored as a single
// comma-separated string, since the property model has no dedicated
// list-editing UI yet. Values may be left empty, in which case the output is
// just the selected index (0, 1, 2, ...).
class PHOTONCORE_EXPORT SegmentedGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    SegmentedGizmo();

    QString text() const;
    void setText(const QString &);

    QString optionsText() const;
    void setOptionsText(const QString &);
    QStringList optionList() const;

    QString valuesText() const;
    void setValuesText(const QString &);
    QStringList valueList() const;

    int selectedIndex() const;
    void setSelectedIndex(int);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;
};

} // namespace photon

#endif // SEGMENTEDGIZMO_H

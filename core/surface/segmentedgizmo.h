#ifndef SEGMENTEDGIZMO_H
#define SEGMENTEDGIZMO_H
#include <QStringList>
#include "surfacegizmo.h"

namespace photon {

// A segmented control — a row (or column) of mutually-exclusive options, like
// a set of radio buttons. Options are stored as a single comma-separated
// string since the property model has no dedicated list-editing UI yet.
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

    int selectedIndex() const;
    void setSelectedIndex(int);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;
};

} // namespace photon

#endif // SEGMENTEDGIZMO_H

#ifndef KEIRA_PARAMETERRANGETEST_H
#define KEIRA_PARAMETERRANGETEST_H

#include <QObject>

namespace keira {

// Covers the hard-vs-soft bounds split on DecimalParameter and
// IntegerParameter: the "mirror until explicitly narrowed" default, that
// setSoftRange/setMinimum/setMaximum don't clobber each other regardless of
// call order, and the JSON round trip pinned tabs and saved graphs depend on.
//
// NumberScrubField itself (the QWidget the soft range actually drives the
// fill bar and drag/wheel feel of) is left to manual verification, consistent
// with this project's existing test scope for interactive widgets.
class ParameterRangeTest : public QObject
{
    Q_OBJECT
public:
    explicit ParameterRangeTest(QObject *parent = nullptr);

private slots:
    void decimalSoftRangeMirrorsHardByDefault();
    void decimalSoftRangeIndependentOfHard();
    void decimalHardRangeChangeDoesNotClobberExplicitSoftRange();
    void decimalJsonRoundTripPreservesSoftRange();
    void decimalJsonOmitsSoftRangeWhenNeverSet();

    void integerSoftRangeMirrorsHardByDefault();
    void integerSoftRangeIndependentOfHard();
    void integerJsonRoundTripPreservesSoftRange();
};

} // namespace keira

#endif // KEIRA_PARAMETERRANGETEST_H

#ifndef PHOTONUI_NUMBERSCRUBFIELD_H
#define PHOTONUI_NUMBERSCRUBFIELD_H

#include <QWidget>
#include <QPoint>
#include "photon-ui-global.h"

class QLineEdit;

namespace photon {

// A modern numeric field: click to type, drag horizontally to scrub the value,
// with a fill bar showing where the value sits in a bounded range. Used by both
// DecimalParameter and IntegerParameter.
//
//  - drag left/right to scrub (Shift = fine 0.1x, Ctrl = coarse 10x)
//  - a plain click (no drag) opens an inline editor for typing
//  - wheel / arrow keys nudge by one step
//  - bounded [min,max] params map the field width to the full range and draw a
//    fill; unbounded params scrub at a fixed step-per-pixel with no fill
//
// Hard bounds vs. soft bounds: the *hard* range (setRange/setMinimum/
// setMaximum) is the absolute ceiling - it clamps every value the field can
// ever hold, however it got there (typed, scrubbed, wheeled, setValue()). The
// *soft* range (setSoftRange/setSoftMinimum/setSoftMaximum) is only the
// interactive range: it's what the fill bar and drag/wheel/arrow-key
// sensitivity are scaled to, so the field "feels" bounded to its expected
// range without refusing a value outside it. A hue parameter is the
// motivating case - soft range 0-1 for a comfortable slider, hard range left
// open so a value can still be typed (or animated) past 1 and wrap.
//
// By default the soft range mirrors whatever the hard range is, so a field
// that never calls setSoftRange behaves exactly as it did before soft bounds
// existed. Call setSoftRange to narrow the interactive range independent of
// the hard one; the two can be set in either order without one clobbering
// the other; needed since setRange is a fixed contract used by ~30 existing
// callers that never actually need it.
class PHOTONUI_EXPORT NumberScrubField : public QWidget
{
    Q_OBJECT
public:
    explicit NumberScrubField(QWidget *parent = nullptr);
    ~NumberScrubField() override;

    void setIsInteger(bool);

    // Hard bounds - the absolute ceiling on the value (see class comment).
    // Also becomes the soft range, unless setSoftRange has already narrowed it.
    void setRange(double minimum, double maximum);
    void setMinimum(double);   // convenience: keeps the current maximum
    void setMaximum(double);   // convenience: keeps the current minimum
    double minimum() const;
    double maximum() const;

    // Soft bounds - the interactive range (see class comment). Independent of
    // the hard range; safe to call before or after setRange.
    void setSoftRange(double minimum, double maximum);
    void setSoftMinimum(double);   // convenience: keeps the current soft maximum
    void setSoftMaximum(double);   // convenience: keeps the current soft minimum
    double softMinimum() const;
    double softMaximum() const;
    // True once setSoftRange/setSoftMinimum/setSoftMaximum has been called -
    // i.e. the soft range no longer just mirrors the hard one.
    bool hasExplicitSoftRange() const;

    void setDecimals(int);
    void setReadOnly(bool);

    double value() const;

    QSize sizeHint() const override;

public slots:
    void setValue(double);

signals:
    void valueChanged(double);      // live during scrub/typing
    void editingFinished();         // scrub ended or text committed

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

private:
    // Whether the *soft* range is finite - what actually gates the fill bar
    // and the interactive scrub/wheel/arrow-key sensitivity.
    bool isSoftBounded() const;
    double clamp(double) const;   // hard-bound clamp; applies to every settable value
    double step() const;
    QString formatted(double) const;
    void beginEdit();
    void commitEdit();
    void cancelEdit();

    QLineEdit *m_edit = nullptr;
    double m_value = 0.0;
    double m_hardMinimum;
    double m_hardMaximum;
    double m_softMinimum;
    double m_softMaximum;
    bool m_softExplicit = false;   // false: m_soft* just mirrors m_hard* on every setRange()
    int m_decimals = 4;
    bool m_isInteger = false;
    bool m_readOnly = false;

    // scrub state
    bool m_pressed = false;
    bool m_scrubbing = false;
    QPoint m_pressGlobal;
    QPoint m_lastGlobal;
    double m_scrubAccum = 0.0;
    bool m_hovered = false;
};

} // namespace photon

#endif // PHOTONUI_NUMBERSCRUBFIELD_H

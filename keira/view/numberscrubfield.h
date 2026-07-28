#ifndef KEIRA_NUMBERSCRUBFIELD_H
#define KEIRA_NUMBERSCRUBFIELD_H

#include <QWidget>
#include <QPoint>
#include "keira-global.h"

class QLineEdit;

namespace keira {

// A modern numeric field: click to type, drag horizontally to scrub the value,
// with a fill bar showing where the value sits in a bounded range. Used by both
// DecimalParameter and IntegerParameter.
//
//  - drag left/right to scrub (Shift = fine 0.1x, Ctrl = coarse 10x)
//  - a plain click (no drag) opens an inline editor for typing
//  - wheel / arrow keys nudge by one step
//  - bounded [min,max] params map the field width to the full range and draw a
//    fill; unbounded params scrub at a fixed step-per-pixel with no fill
class KEIRA_EXPORT NumberScrubField : public QWidget
{
    Q_OBJECT
public:
    explicit NumberScrubField(QWidget *parent = nullptr);
    ~NumberScrubField() override;

    void setIsInteger(bool);
    void setRange(double minimum, double maximum);
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
    bool isBounded() const;
    double clamp(double) const;
    double step() const;
    QString formatted(double) const;
    void beginEdit();
    void commitEdit();
    void cancelEdit();

    QLineEdit *m_edit = nullptr;
    double m_value = 0.0;
    double m_minimum;
    double m_maximum;
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

} // namespace keira

#endif // KEIRA_NUMBERSCRUBFIELD_H

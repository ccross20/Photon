#ifndef PHOTON_CLIPPROPERTYEDITOR_H
#define PHOTON_CLIPPROPERTYEDITOR_H

#include <QPointer>
#include <QWidget>
#include "photon-global.h"

class QComboBox;
class QLabel;
class QLineEdit;

namespace photon {

class ColorWheelSwatch;
class NumberScrubField;

// Properties panel page for a sequence clip: name, color, timing, strength
// and easing. Follows edits made in the timeline while it's open.
class ClipPropertyEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ClipPropertyEditor(Clip *clip, QWidget *parent = nullptr);

private:
    void refresh();

    QPointer<Clip> m_clip;

    QLineEdit *m_name;
    QLabel *m_type;
    QLabel *m_layer;
    ColorWheelSwatch *m_color;
    NumberScrubField *m_start;
    NumberScrubField *m_end;
    QLabel *m_duration;
    NumberScrubField *m_strength;
    QComboBox *m_easeInType;
    NumberScrubField *m_easeInDuration;
    QComboBox *m_easeOutType;
    NumberScrubField *m_easeOutDuration;
};

} // namespace photon

#endif // PHOTON_CLIPPROPERTYEDITOR_H

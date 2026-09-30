#ifndef PHOTON_COLORCALIBRATIONDIALOG_H
#define PHOTON_COLORCALIBRATIONDIALOG_H

#include <QDialog>
#include "photon-global.h"

namespace photon {

struct ColorCalibrationEntry;

// Lets the user calibrate how this fixture's color-mixing LEDs (Red, Green,
// Blue, Amber, Lime, White, ...) combine to reproduce a set of named
// reference hues: pick a hue button, drag the per-channel sliders until the
// real fixture (driven live through the project's "Output Overrides" bus
// node) matches, then Save. Calibration attaches to the fixture's
// definition path, so it's shared by every patched fixture of that type.
class PHOTONCORE_EXPORT ColorCalibrationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColorCalibrationDialog(Fixture *fixture, QWidget *parent = nullptr);
    ~ColorCalibrationDialog();

private slots:
    void selectHue(int index);
    void sliderChanged();
    void fixturePickerChanged(int index);
    void save();
    void cancel();

private:
    void commitActiveHue();
    void loadSlidersFromEntry(const ColorCalibrationEntry &);
    void pushLivePreview();
    void stopPreview();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORCALIBRATIONDIALOG_H

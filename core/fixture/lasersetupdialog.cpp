#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include "lasersetupdialog.h"
#include "fixture.h"
#include "numberscrubfield.h"

namespace photon {

// The FB4 only enters its setup profile after the mode channel has held the
// setup value for this long.
static constexpr qint64 SetupEntryMs = 2000;

class LaserSetupDialog::Impl
{
public:
    QPointer<Fixture> fixture;
    Fixture::LaserSetup original;
    QElapsedTimer sinceOpened;
    QTimer statusTimer;
    bool suppressPush = false;

    QLabel *statusLabel;
    NumberScrubField *intensity;
    NumberScrubField *testFrame;
    NumberScrubField *sizeX;
    NumberScrubField *sizeY;
    NumberScrubField *positionX;
    NumberScrubField *positionY;
    NumberScrubField *rotation;
};

static NumberScrubField *makeField(double t_minimum, double t_maximum, int t_decimals)
{
    auto *field = new NumberScrubField;
    field->setRange(t_minimum, t_maximum);
    field->setDecimals(t_decimals);
    if(t_decimals == 0)
        field->setIsInteger(true);
    return field;
}

LaserSetupDialog::LaserSetupDialog(Fixture *t_fixture, QWidget *t_parent)
    : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Laser Setup — " + t_fixture->name());

    m_impl->fixture = t_fixture;
    m_impl->original = t_fixture->laserSetup();

    auto *layout = new QVBoxLayout;

    auto *warning = new QLabel("While this window is open the laser is held in setup mode. "
                               "Test frames project real laser output - check the projection area is clear.");
    warning->setWordWrap(true);
    layout->addWidget(warning);

    m_impl->statusLabel = new QLabel;
    layout->addWidget(m_impl->statusLabel);

    auto *form = new QFormLayout;
    m_impl->intensity = makeField(0.0, 100.0, 0);
    m_impl->testFrame = makeField(0.0, 255.0, 0);
    m_impl->sizeX = makeField(-100.0, 100.0, 1);
    m_impl->sizeY = makeField(-100.0, 100.0, 1);
    m_impl->positionX = makeField(-100.0, 100.0, 1);
    m_impl->positionY = makeField(-100.0, 100.0, 1);
    m_impl->rotation = makeField(0.0, 360.0, 1);

    form->addRow("Master Intensity (%)", m_impl->intensity);
    form->addRow("Test Frame (0 = off)", m_impl->testFrame);
    const QString sizeTip = "100% = full size, 0% = collapsed, negative = mirrored";
    m_impl->sizeX->setToolTip(sizeTip);
    m_impl->sizeY->setToolTip(sizeTip);
    form->addRow("Size X (%)", m_impl->sizeX);
    form->addRow("Size Y (%)", m_impl->sizeY);
    form->addRow("Position X (%)", m_impl->positionX);
    form->addRow("Position Y (%)", m_impl->positionY);
    form->addRow("Rotation (°)", m_impl->rotation);
    layout->addLayout(form);

    m_impl->suppressPush = true;
    m_impl->intensity->setValue(m_impl->original.masterIntensity * 100.0);
    m_impl->testFrame->setValue(m_impl->original.testFrame);
    m_impl->sizeX->setValue(m_impl->original.sizeX * 100.0);
    m_impl->sizeY->setValue(m_impl->original.sizeY * 100.0);
    m_impl->positionX->setValue(m_impl->original.positionX * 100.0);
    m_impl->positionY->setValue(m_impl->original.positionY * 100.0);
    m_impl->rotation->setValue(m_impl->original.rotation);
    m_impl->suppressPush = false;

    for(auto *field : {m_impl->intensity, m_impl->testFrame, m_impl->sizeX, m_impl->sizeY,
                        m_impl->positionX, m_impl->positionY, m_impl->rotation})
        connect(field, &NumberScrubField::valueChanged, this, &LaserSetupDialog::pushValues);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &LaserSetupDialog::resetValues);
    layout->addWidget(buttons);

    setLayout(layout);

    t_fixture->setLaserSetupActive(true);
    m_impl->sinceOpened.start();
    connect(&m_impl->statusTimer, &QTimer::timeout, this, &LaserSetupDialog::updateStatus);
    m_impl->statusTimer.start(100);
    updateStatus();
}

LaserSetupDialog::~LaserSetupDialog()
{
    if(m_impl->fixture)
        m_impl->fixture->setLaserSetupActive(false);
    delete m_impl;
}

void LaserSetupDialog::done(int t_result)
{
    m_impl->statusTimer.stop();
    if(m_impl->fixture)
    {
        if(t_result != QDialog::Accepted)
            m_impl->fixture->setLaserSetup(m_impl->original);
        m_impl->fixture->setLaserSetupActive(false);
    }
    QDialog::done(t_result);
}

void LaserSetupDialog::pushValues()
{
    if(m_impl->suppressPush || !m_impl->fixture)
        return;

    Fixture::LaserSetup setup;
    setup.masterIntensity = m_impl->intensity->value() / 100.0;
    setup.testFrame = int(m_impl->testFrame->value());
    setup.sizeX = m_impl->sizeX->value() / 100.0;
    setup.sizeY = m_impl->sizeY->value() / 100.0;
    setup.positionX = m_impl->positionX->value() / 100.0;
    setup.positionY = m_impl->positionY->value() / 100.0;
    setup.rotation = m_impl->rotation->value();
    m_impl->fixture->setLaserSetup(setup);
}

void LaserSetupDialog::resetValues()
{
    const Fixture::LaserSetup defaults;
    m_impl->suppressPush = true;
    m_impl->intensity->setValue(defaults.masterIntensity * 100.0);
    m_impl->testFrame->setValue(defaults.testFrame);
    m_impl->sizeX->setValue(defaults.sizeX * 100.0);
    m_impl->sizeY->setValue(defaults.sizeY * 100.0);
    m_impl->positionX->setValue(defaults.positionX * 100.0);
    m_impl->positionY->setValue(defaults.positionY * 100.0);
    m_impl->rotation->setValue(defaults.rotation);
    m_impl->suppressPush = false;
    pushValues();
}

void LaserSetupDialog::updateStatus()
{
    if(!m_impl->fixture)
    {
        m_impl->statusLabel->setText("Fixture was removed.");
        m_impl->statusTimer.stop();
        return;
    }

    const qint64 remaining = SetupEntryMs - m_impl->sinceOpened.elapsed();
    if(remaining > 0)
    {
        m_impl->statusLabel->setText(QString("Entering setup mode… %1 s").arg(remaining / 1000.0, 0, 'f', 1));
        return;
    }

    m_impl->statusLabel->setText("<b>In setup mode</b> — changes are sent live.");
    m_impl->statusTimer.stop();
}

} // namespace photon

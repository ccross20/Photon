#include <algorithm>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "colorcalibrationdialog.h"
#include "fixture.h"
#include "fixturecollection.h"
#include "capability/colorcapability.h"
#include "capability/fixturecolorcalibration.h"
#include "graph/bus/outputoverridesnode.h"
#include "graph/bus/busgraph.h"
#include "project/project.h"
#include "photoncore.h"
#include "numberscrubfield.h"
#include "tag/tagcolor.h"

namespace photon {

namespace {

struct HueSlot
{
    QString name;
    double hueDegrees;
    bool isWhite;
};

const QVector<HueSlot> &hueSlots()
{
    static const QVector<HueSlot> kHueSlots = {
        {"Red", 0.0, false},
        {"Orange", 30.0, false},
        {"Yellow", 60.0, false},
        {"Green", 120.0, false},
        {"Cyan", 180.0, false},
        {"Blue", 240.0, false},
        {"Magenta", 300.0, false},
        {"Cool White", 0.0, true},
        {"Warm White", 0.0, true},
    };
    return kHueSlots;
}

QString capitalized(const QString &t_text)
{
    if(t_text.isEmpty())
        return t_text;
    return t_text.left(1).toUpper() + t_text.mid(1);
}

} // namespace

class ColorCalibrationDialog::Impl
{
public:
    Fixture *fixture = nullptr;
    ColorCapability *colorCap = nullptr;
    FixtureColorCalibration calibration;
    OutputOverridesNode *previewNode = nullptr;
    int activeSlot = -1;
    bool suppressSliders = false;

    QButtonGroup *hueGroup;
    QComboBox *fixturePicker;
    QMap<CapabilityType, NumberScrubField*> sliders;
    QVBoxLayout *sliderLayout;
};

ColorCalibrationDialog::ColorCalibrationDialog(Fixture *t_fixture, QWidget *t_parent)
    : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Calibrate Colors — " + t_fixture->name());

    m_impl->fixture = t_fixture;
    m_impl->colorCap = t_fixture->color();
    m_impl->calibration = FixtureColorCalibrationStore::load(t_fixture->definitionPath());

    Project *project = photonApp->project();
    if(project)
        m_impl->previewNode = OutputOverridesNode::find(project->bus());

    auto *layout = new QVBoxLayout;

    // Fixture picker — which patched instance of this fixture type to drive live.
    auto *pickerLayout = new QHBoxLayout;
    pickerLayout->addWidget(new QLabel("Preview On"));
    m_impl->fixturePicker = new QComboBox;
    if(project)
    {
        for(auto *candidate : project->fixtures()->fixtures())
        {
            if(candidate->definitionPath() != t_fixture->definitionPath())
                continue;
            m_impl->fixturePicker->addItem(candidate->name(), candidate->uniqueId());
        }
    }
    const int currentFixtureIndex = m_impl->fixturePicker->findData(t_fixture->uniqueId());
    m_impl->fixturePicker->setCurrentIndex(currentFixtureIndex >= 0 ? currentFixtureIndex : 0);
    pickerLayout->addWidget(m_impl->fixturePicker, 1);
    layout->addLayout(pickerLayout);

    // Hue button row.
    auto *hueLayout = new QHBoxLayout;
    m_impl->hueGroup = new QButtonGroup(this);
    m_impl->hueGroup->setExclusive(true);

    const auto &hueSlotList = hueSlots();
    for(int i = 0; i < hueSlotList.length(); ++i)
    {
        if(i == hueSlotList.length() - 2)
        {
            auto *separator = new QFrame;
            separator->setFrameShape(QFrame::VLine);
            hueLayout->addWidget(separator);
        }

        const HueSlot &slot = hueSlotList[i];
        const QColor swatch = slot.isWhite ? QColor(Qt::white) : QColor::fromHsv(int(slot.hueDegrees), 255, 255);

        auto *button = new QPushButton(slot.name);
        button->setCheckable(true);
        button->setStyleSheet(QStringLiteral("QPushButton { background-color: %1; color: %2; }")
                                   .arg(swatch.name(), tagTextColor(swatch).name()));
        m_impl->hueGroup->addButton(button, i);
        hueLayout->addWidget(button);
    }
    layout->addLayout(hueLayout);

    // Per-channel sliders — one per LED type this fixture's color capability drives.
    m_impl->sliderLayout = new QVBoxLayout;
    auto *sliderForm = new QFormLayout;
    if(m_impl->colorCap)
    {
        QVector<CapabilityType> seen;
        for(auto type : m_impl->colorCap->channelTypes())
        {
            if(seen.contains(type) || colorChannelTypeName(type).isEmpty())
                continue;
            seen.append(type);

            auto *slider = new NumberScrubField;
            slider->setRange(0.0, 1.0);
            slider->setValue(0.0);
            slider->setMinimumWidth(270);
            m_impl->sliders.insert(type, slider);
            sliderForm->addRow(capitalized(colorChannelTypeName(type)), slider);

            connect(slider, &NumberScrubField::valueChanged, this, &ColorCalibrationDialog::sliderChanged);
        }
    }
    m_impl->sliderLayout->addLayout(sliderForm);
    layout->addLayout(m_impl->sliderLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &ColorCalibrationDialog::save);
    connect(buttons, &QDialogButtonBox::rejected, this, &ColorCalibrationDialog::cancel);
    layout->addWidget(buttons);

    setLayout(layout);

    connect(m_impl->hueGroup, &QButtonGroup::idClicked, this, &ColorCalibrationDialog::selectHue);
    connect(m_impl->fixturePicker, &QComboBox::currentIndexChanged, this, &ColorCalibrationDialog::fixturePickerChanged);

    if(m_impl->previewNode)
    {
        const QVariant fixtureId = m_impl->fixturePicker->currentData();
        m_impl->previewNode->setCalibrationFixture(fixtureId.toByteArray());
        m_impl->previewNode->setCalibrationPreviewEnabled(true);
    }

    if(!hueSlotList.isEmpty())
        m_impl->hueGroup->button(0)->click();
}

ColorCalibrationDialog::~ColorCalibrationDialog()
{
    stopPreview();
    delete m_impl;
}

void ColorCalibrationDialog::loadSlidersFromEntry(const ColorCalibrationEntry &t_entry)
{
    m_impl->suppressSliders = true;
    for(auto it = m_impl->sliders.cbegin(); it != m_impl->sliders.cend(); ++it)
        it.value()->setValue(t_entry.channelPercents.value(it.key(), 0.0));
    m_impl->suppressSliders = false;
}

void ColorCalibrationDialog::commitActiveHue()
{
    if(m_impl->activeSlot < 0)
        return;

    const HueSlot &slot = hueSlots()[m_impl->activeSlot];

    ColorCalibrationEntry entry;
    entry.name = slot.name;
    entry.hueDegrees = slot.hueDegrees;
    for(auto it = m_impl->sliders.cbegin(); it != m_impl->sliders.cend(); ++it)
        entry.channelPercents.insert(it.key(), it.value()->value());

    if(slot.name == "Cool White")
    {
        m_impl->calibration.coolWhite = entry;
        return;
    }
    if(slot.name == "Warm White")
    {
        m_impl->calibration.warmWhite = entry;
        return;
    }

    auto &hues = m_impl->calibration.hues;
    bool replaced = false;
    for(auto &existing : hues)
    {
        if(existing.name == slot.name)
        {
            existing = entry;
            replaced = true;
            break;
        }
    }
    if(!replaced)
        hues.append(entry);

    std::sort(hues.begin(), hues.end(), [](const ColorCalibrationEntry &a, const ColorCalibrationEntry &b){
        return a.hueDegrees < b.hueDegrees;
    });
}

void ColorCalibrationDialog::selectHue(int t_index)
{
    commitActiveHue();
    m_impl->activeSlot = t_index;

    const HueSlot &slot = hueSlots()[t_index];

    const ColorCalibrationEntry *saved = nullptr;
    if(slot.name == "Cool White" && m_impl->calibration.coolWhite.isValid())
        saved = &m_impl->calibration.coolWhite;
    else if(slot.name == "Warm White" && m_impl->calibration.warmWhite.isValid())
        saved = &m_impl->calibration.warmWhite;
    else
    {
        for(const auto &existing : m_impl->calibration.hues)
        {
            if(existing.name == slot.name)
            {
                saved = &existing;
                break;
            }
        }
    }

    if(saved)
    {
        loadSlidersFromEntry(*saved);
    }
    else
    {
        // Naive starting point — a straight RGB guess (or full white for the
        // white slots) so the user has somewhere to start tuning from.
        ColorCalibrationEntry fallback;
        fallback.name = slot.name;
        fallback.hueDegrees = slot.hueDegrees;
        if(slot.isWhite)
        {
            if(m_impl->colorCap && m_impl->colorCap->hasWhite())
                fallback.channelPercents.insert(Capability_White, 1.0);
            else
            {
                fallback.channelPercents.insert(Capability_Red, 1.0);
                fallback.channelPercents.insert(Capability_Green, 1.0);
                fallback.channelPercents.insert(Capability_Blue, 1.0);
            }
        }
        else
        {
            const QColor guess = QColor::fromHsv(int(slot.hueDegrees), 255, 255);
            fallback.channelPercents.insert(Capability_Red, guess.redF());
            fallback.channelPercents.insert(Capability_Green, guess.greenF());
            fallback.channelPercents.insert(Capability_Blue, guess.blueF());
        }
        loadSlidersFromEntry(fallback);
    }

    pushLivePreview();
}

void ColorCalibrationDialog::sliderChanged()
{
    if(m_impl->suppressSliders)
        return;
    pushLivePreview();
}

void ColorCalibrationDialog::fixturePickerChanged(int)
{
    if(m_impl->previewNode)
        m_impl->previewNode->setCalibrationFixture(m_impl->fixturePicker->currentData().toByteArray());
}

void ColorCalibrationDialog::pushLivePreview()
{
    if(!m_impl->previewNode)
        return;

    m_impl->previewNode->clearCalibrationChannelPercents();
    for(auto it = m_impl->sliders.cbegin(); it != m_impl->sliders.cend(); ++it)
        m_impl->previewNode->setCalibrationChannelPercent(it.key(), it.value()->value());
    m_impl->previewNode->setCalibrationFixture(m_impl->fixturePicker->currentData().toByteArray());
    m_impl->previewNode->setCalibrationPreviewEnabled(true);
}

void ColorCalibrationDialog::stopPreview()
{
    if(m_impl->previewNode)
        m_impl->previewNode->setCalibrationPreviewEnabled(false);
}

void ColorCalibrationDialog::save()
{
    commitActiveHue();
    stopPreview();
    FixtureColorCalibrationStore::save(m_impl->fixture->definitionPath(), m_impl->calibration);
    for(int i = 0; i < m_impl->fixture->colorCount(); ++i)
        m_impl->fixture->colorAtIndex(i)->setCalibration(m_impl->calibration);
    accept();
}

void ColorCalibrationDialog::cancel()
{
    stopPreview();
    reject();
}

} // namespace photon

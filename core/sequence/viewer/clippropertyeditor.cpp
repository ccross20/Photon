#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QToolButton>
#include "clippropertyeditor.h"
#include "color/colorwheelswatch.h"
#include "numberscrubfield.h"
#include "propertycombobox.h"
#include "propertywidgets.h"
#include "plugin/pluginfactory.h"
#include "sequence/clip.h"
#include "sequence/cliplayer.h"
#include "photoncore.h"

namespace photon {

namespace {

// Matches the timeline's own default clip color (SequenceClip::paint).
const QColor kDefaultClipColor(Qt::red);
constexpr double kMinDuration = 0.01;

// Same choices as the clip's right-click Ease In/Out menus.
const QVector<QPair<QString, QEasingCurve::Type>> &easeTypes()
{
    static const QVector<QPair<QString, QEasingCurve::Type>> types = {
        {"Linear", QEasingCurve::Linear},
        {"In Out Curve", QEasingCurve::InOutCubic},
        {"In Curve", QEasingCurve::InCubic},
        {"Out Curve", QEasingCurve::OutCubic},
    };
    return types;
}

QComboBox *makeEaseCombo()
{
    auto *combo = new PropertyComboBox;
    for(const auto &type : easeTypes())
        combo->addItem(type.first, int(type.second));
    return combo;
}

void selectEaseType(QComboBox *t_combo, QEasingCurve::Type t_type)
{
    int index = t_combo->findData(int(t_type));
    if(index < 0)
    {
        // A curve set some other way (older files); show it rather than lie.
        t_combo->addItem("Custom", int(t_type));
        index = t_combo->count() - 1;
    }
    t_combo->setCurrentIndex(index);
}

NumberScrubField *makeSecondsField()
{
    auto *field = new NumberScrubField;
    field->setDecimals(3);
    field->setMinimum(0.0);
    return field;
}

QString clipTypeName(const Clip *t_clip)
{
    for(const ClipInformation &info : photonApp->plugins()->clips())
        if(info.id == t_clip->id())
            return info.name;
    return QString::fromLatin1(t_clip->id());
}

} // namespace

ClipPropertyEditor::ClipPropertyEditor(Clip *t_clip, QWidget *t_parent)
    : QWidget(t_parent), m_clip(t_clip)
{
    // Same page layout as every other property editor (fixtures, nodes, ...).
    auto *form = new PropertyForm;
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(form);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    form->addSection("General");
    m_name = new QLineEdit;
    form->addRow("Name", m_name);
    m_type = new QLabel;
    form->addRow("Type", m_type);
    m_layer = new QLabel;
    form->addRow("Layer", m_layer);

    auto *colorRow = new QWidget;
    auto *colorLayout = new QHBoxLayout(colorRow);
    colorLayout->setContentsMargins(0, 0, 0, 0);
    m_color = new ColorWheelSwatch(kDefaultClipColor);
    colorLayout->addWidget(m_color, 1);
    auto *resetColor = new QToolButton;
    resetColor->setText("Default");
    resetColor->setToolTip("Use the default clip color");
    colorLayout->addWidget(resetColor);
    form->addRow("Color", colorRow);

    form->addSection("Timing");
    m_start = makeSecondsField();
    form->addRow("Start (s)", m_start);
    m_end = makeSecondsField();
    form->addRow("End (s)", m_end);
    m_duration = new QLabel;
    form->addRow("Duration (s)", m_duration);
    m_strength = new NumberScrubField;
    m_strength->setDecimals(2);
    m_strength->setRange(0.0, 1.0);
    form->addRow("Strength", m_strength);

    form->addSection("Ease In");
    m_easeInType = makeEaseCombo();
    form->addRow("Curve", m_easeInType);
    m_easeInDuration = makeSecondsField();
    form->addRow("Duration (s)", m_easeInDuration);

    form->addSection("Ease Out");
    m_easeOutType = makeEaseCombo();
    form->addRow("Curve", m_easeOutType);
    m_easeOutDuration = makeSecondsField();
    form->addRow("Duration (s)", m_easeOutDuration);
    form->addStretch();

    connect(m_name, &QLineEdit::editingFinished, this, [this](){
        if(m_clip)
            m_clip->setName(m_name->text());
    });
    connect(m_color, &ColorWheelSwatch::colorChanged, this, [this](const QColor &color){
        if(m_clip)
            m_clip->setColor(color);
    });
    connect(resetColor, &QToolButton::clicked, this, [this](){
        if(m_clip)
            m_clip->setColor(QColor());
    });

    // Start and end each move one edge; the other edge stays where it is.
    connect(m_start, &NumberScrubField::valueChanged, this, [this](double start){
        if(!m_clip)
            return;
        const double end = m_clip->endTime();
        start = std::min(start, end - kMinDuration);
        m_clip->setStartTime(start);
        m_clip->setDuration(end - start);
    });
    connect(m_end, &NumberScrubField::valueChanged, this, [this](double end){
        if(m_clip)
            m_clip->setDuration(std::max(kMinDuration, end - m_clip->startTime()));
    });
    connect(m_strength, &NumberScrubField::valueChanged, this, [this](double value){
        if(m_clip)
            m_clip->setStrength(value);
    });
    connect(m_easeInType, &QComboBox::currentIndexChanged, this, [this](){
        if(m_clip)
            m_clip->setEaseInType(QEasingCurve::Type(m_easeInType->currentData().toInt()));
    });
    connect(m_easeOutType, &QComboBox::currentIndexChanged, this, [this](){
        if(m_clip)
            m_clip->setEaseOutType(QEasingCurve::Type(m_easeOutType->currentData().toInt()));
    });
    connect(m_easeInDuration, &NumberScrubField::valueChanged, this, [this](double value){
        if(m_clip)
            m_clip->setEaseInDuration(value);
    });
    connect(m_easeOutDuration, &NumberScrubField::valueChanged, this, [this](double value){
        if(m_clip)
            m_clip->setEaseOutDuration(value);
    });

    if(m_clip)
        connect(m_clip, &Clip::clipUpdated, this, &ClipPropertyEditor::refresh);
    refresh();
}

void ClipPropertyEditor::refresh()
{
    if(!m_clip)
    {
        setEnabled(false);
        return;
    }

    // Programmatic updates must not echo back into the clip as edits.
    const QSignalBlocker blockers[] = {
        QSignalBlocker(m_name), QSignalBlocker(m_color), QSignalBlocker(m_start),
        QSignalBlocker(m_end), QSignalBlocker(m_strength),
        QSignalBlocker(m_easeInType), QSignalBlocker(m_easeInDuration),
        QSignalBlocker(m_easeOutType), QSignalBlocker(m_easeOutDuration),
    };

    if(!m_name->hasFocus())
        m_name->setText(m_clip->name());
    m_type->setText(clipTypeName(m_clip));
    m_layer->setText(m_clip->layer() ? m_clip->layer()->name() : QString());
    m_color->setColor(m_clip->color().isValid() ? m_clip->color() : kDefaultClipColor);

    m_start->setValue(m_clip->startTime());
    m_end->setValue(m_clip->endTime());
    m_duration->setText(QString::number(m_clip->duration(), 'f', 3));
    m_strength->setValue(m_clip->strength());

    selectEaseType(m_easeInType, m_clip->easeInType());
    selectEaseType(m_easeOutType, m_clip->easeOutType());
    m_easeInDuration->setMaximum(m_clip->duration());
    m_easeOutDuration->setMaximum(m_clip->duration());
    m_easeInDuration->setValue(m_clip->easeInDuration());
    m_easeOutDuration->setValue(m_clip->easeOutDuration());
}

} // namespace photon

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QFrame>
#include <QScrollArea>
#include <functional>
#include "fixturestateeditor.h"
#include "view/numberscrubfield.h"
#include "gui/color/colorwheelswatch.h"
#include "graph/node/fixture/fixturestatenode.h"
#include "state/state.h"
#include "state/statecapability.h"
#include "sequence/channel.h"
#include "fixture/capability/fixturecapability.h"
#include "fixture/fixture.h"

namespace photon {

// Editor widget for a single capability channel, chosen by its value type.
// t_nameOptions is only used for ChannelTypeString (the fixture channel names
// available for the capability's type, so e.g. a rotation's "Name" field offers a
// dropdown of the actual matching channels instead of free-text entry).
// t_onEdit is invoked after every value change so the node can mark itself dirty
// (the State isn't a keira Parameter, so nothing else does).
static QWidget *makeChannelEditor(StateCapability *t_cap, int t_index, const ChannelInfo &t_info,
                                  const QStringList &t_nameOptions, std::function<void()> t_onEdit)
{
    switch(t_info.type)
    {
    case ChannelInfo::ChannelTypeString:
    {
        auto *combo = new QComboBox;
        combo->setEditable(true);   // fall back to free text if no fixture match yet
        combo->addItems(t_nameOptions);
        const QString current = t_cap->getChannelValue(t_index).toString();
        const int idx = combo->findText(current, Qt::MatchFixedString);
        if(idx >= 0)
            combo->setCurrentIndex(idx);
        else
            combo->setCurrentText(current);
        QObject::connect(combo, &QComboBox::currentTextChanged, combo, [t_cap, t_index, t_onEdit](const QString &v){ t_cap->setChannelValue(t_index, v); t_onEdit(); });
        return combo;
    }
    case ChannelInfo::ChannelTypeColor:
    {
        // Swatch that opens the app's ColorSelectorDialog, same as the node
        // editor's color parameter - not the OS colour picker.
        auto *swatch = new ColorWheelSwatch(t_cap->getChannelValue(t_index).value<QColor>());
        QObject::connect(swatch, &ColorWheelSwatch::colorChanged, swatch, [t_cap, t_index, t_onEdit](const QColor &c){
            t_cap->setChannelValue(t_index, c);
            t_onEdit();
        });
        return swatch;
    }
    case ChannelInfo::ChannelTypeBool:
    {
        auto *chk = new QCheckBox;
        chk->setChecked(t_cap->getChannelValue(t_index).toBool());
        QObject::connect(chk, &QCheckBox::toggled, chk, [t_cap, t_index, t_onEdit](bool v){ t_cap->setChannelValue(t_index, v); t_onEdit(); });
        return chk;
    }
    case ChannelInfo::ChannelTypeInteger:
    case ChannelInfo::ChannelTypeIntegerStep:
    {
        // Same click-to-type / drag-to-scrub field the node editor uses. Typed
        // values commit on Return or focus-out, not on every keystroke; a
        // drag-scrub still updates live.
        auto *field = new keira::NumberScrubField;
        field->setIsInteger(true);
        field->setRange(0, 255);
        field->setValue(t_cap->getChannelValue(t_index).toInt());
        QObject::connect(field, &keira::NumberScrubField::valueChanged, field, [t_cap, t_index, t_onEdit](double v){ t_cap->setChannelValue(t_index, int(v)); t_onEdit(); });
        return field;
    }
    default: // Number and anything else
    {
        auto *field = new keira::NumberScrubField;
        field->setDecimals(3);
        // A channel with a known useful range gets a bounded slider (fill bar,
        // width mapped to the range, typed values clamped); dual-purpose ones
        // like Pan/Tilt (percent or degrees) stay unbounded.
        if(t_info.hasRange())
            field->setRange(t_info.minimum, t_info.maximum);
        field->setValue(t_cap->getChannelValue(t_index).toDouble());
        QObject::connect(field, &keira::NumberScrubField::valueChanged, field, [t_cap, t_index, t_onEdit](double v){ t_cap->setChannelValue(t_index, v); t_onEdit(); });
        return field;
    }
    }
}

FixtureStateEditor::FixtureStateEditor(FixtureStateNode *t_node, QWidget *parent)
    : QWidget(parent), m_node(t_node)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    // The capability frames go in a scroll area so a long list scrolls rather
    // than compressing each frame (the node editor panel isn't itself
    // scrollable). It expands to fill whatever height the panel gives it.
    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    auto *content = new QWidget;
    m_listLayout = new QVBoxLayout(content);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(4);
    m_scroll->setWidget(content);
    outer->addWidget(m_scroll, 1);

    auto *addButton = new QPushButton("Add Capability");
    connect(addButton, &QPushButton::clicked, this, &FixtureStateEditor::openAddMenu);
    outer->addWidget(addButton);

    rebuild();
}

void FixtureStateEditor::rebuild()
{
    QLayoutItem *item;
    while((item = m_listLayout->takeAt(0)) != nullptr)
    {
        if(item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    State *state = m_node ? m_node->state() : nullptr;
    if(state)
    {
        for(auto *cap : state->capabilities())
        {
            auto *frame = new QFrame;
            frame->setFrameShape(QFrame::StyledPanel);
            auto *v = new QVBoxLayout(frame);
            v->setContentsMargins(6, 4, 6, 4);
            v->setSpacing(3);

            auto *header = new QHBoxLayout;
            header->addWidget(new QLabel("<b>" + cap->name() + "</b>"));
            header->addStretch();
            auto *removeBtn = new QPushButton("×");
            removeBtn->setFixedWidth(22);
            connect(removeBtn, &QPushButton::clicked, this, [this, state, cap](){
                state->removeCapability(cap);
                delete cap;
                if(m_node)
                    m_node->markStateEdited();
                rebuild();
            });
            header->addWidget(removeBtn);
            v->addLayout(header);

            const auto channels = cap->availableChannels();
            for(int i = 0; i < channels.size(); ++i)
            {
                auto *row = new QHBoxLayout;
                row->setSpacing(4);
                row->addWidget(new QLabel(channels[i].name));

                QStringList nameOptions;
                if(channels[i].type == ChannelInfo::ChannelTypeString)
                {
                    for(Fixture *fx : m_node->resolvedFixtures())
                        for(const QString &n : fx->channelNamesForCapability(cap->fixtureCapabilityType()))
                            if(!nameOptions.contains(n, Qt::CaseInsensitive))
                                nameOptions.append(n);
                }

                auto *node = m_node;
                auto *editor = makeChannelEditor(cap, i, channels[i], nameOptions,
                                                 [node](){ if(node) node->markStateEdited(); });
                // Let the editor shrink freely so a narrow panel squeezes it
                // rather than clipping the expose checkbox off the right edge.
                editor->setMinimumWidth(0);
                if(auto *combo = qobject_cast<QComboBox *>(editor))
                {
                    combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
                    combo->setMinimumContentsLength(3);
                }
                row->addWidget(editor, 1);

                // Expose the channel as a graph input port; static editor is
                // disabled while exposed (its value comes from the connection).
                const bool exposed = m_node->isChannelExposed(cap, i);
                auto *exposeCheck = new QCheckBox("→");
                exposeCheck->setToolTip("Expose as graph input");
                exposeCheck->setChecked(exposed);
                editor->setEnabled(!exposed);
                connect(exposeCheck, &QCheckBox::toggled, this, [this, cap, i, editor](bool on){
                    m_node->setChannelExposed(cap, i, on);
                    editor->setEnabled(!on);
                });
                row->addWidget(exposeCheck);

                v->addLayout(row);
            }

            m_listLayout->addWidget(frame);
        }
    }

    // Keep the frames packed at the top; the stretch absorbs any extra height
    // so they stay at their natural size instead of spreading to fill.
    m_listLayout->addStretch();
}

void FixtureStateEditor::openAddMenu()
{
    State *state = m_node ? m_node->state() : nullptr;
    if(!state)
        return;

    // Kept in step with the clip state editor's menu (sequence/viewer/stateeditor.cpp)
    // so both offer the same capabilities. Every entry here must have a case in
    // State::Impl::addCapability, or addCapability returns null and nothing happens.
    struct Entry { const char *name; CapabilityType type; };
    static const Entry entries[] = {
        {"Dimmer", Capability_Dimmer},
        {"Color",  Capability_Color},
        {"Pan",    Capability_Pan},
        {"Tilt",   Capability_Tilt},
        {"Strobe", Capability_Strobe},
        {"Focus",  Capability_Focus},
        {"Zoom",   Capability_Zoom},
        {"Color Slot", Capability_ColorWheelSlot},
        {"Gobo Slot", Capability_WheelSlot},
        {"Wheel Rotation", Capability_WheelRotation},
        {"Wheel Slot Rotation", Capability_WheelSlotRotation},
        {"Lens Rotation", Capability_LensRotation},
        {"Prism", Capability_Prism},
    };

    QMenu menu;
    for(const auto &entry : entries)
    {
        const CapabilityType type = entry.type;
        menu.addAction(entry.name, this, [this, state, type](){
            state->addCapability(type);
            if(m_node)
                m_node->markStateEdited();
            rebuild();
        });
    }
    menu.exec(QCursor::pos());
}

} // namespace photon

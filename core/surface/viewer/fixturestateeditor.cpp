#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QFrame>
#include <QScrollArea>
#include <functional>
#include "fixturestateeditor.h"
#include "numberscrubfield.h"
#include "color/colorwheelswatch.h"
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
        auto *field = new photon::NumberScrubField;
        field->setIsInteger(true);
        field->setRange(0, 255);
        field->setValue(t_cap->getChannelValue(t_index).toInt());
        QObject::connect(field, &photon::NumberScrubField::valueChanged, field, [t_cap, t_index, t_onEdit](double v){ t_cap->setChannelValue(t_index, int(v)); t_onEdit(); });
        return field;
    }
    default: // Number and anything else
    {
        auto *field = new photon::NumberScrubField;
        field->setDecimals(3);
        // A channel with a known useful range gets a bounded slider (fill bar,
        // width mapped to the range, typed values clamped); dual-purpose ones
        // like Pan/Tilt (percent or degrees) stay unbounded.
        if(t_info.hasRange())
            field->setRange(t_info.minimum, t_info.maximum);
        field->setValue(t_cap->getChannelValue(t_index).toDouble());
        QObject::connect(field, &photon::NumberScrubField::valueChanged, field, [t_cap, t_index, t_onEdit](double v){ t_cap->setChannelValue(t_index, v); t_onEdit(); });
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
            v->setContentsMargins(4, 2, 4, 2);
            v->setSpacing(2);

            // Compact header: small flat buttons so the row is only as tall as
            // its label.
            auto *header = new QHBoxLayout;
            header->setContentsMargins(0, 0, 0, 0);
            header->setSpacing(2);
            auto *collapseBtn = new QToolButton;
            collapseBtn->setAutoRaise(true);
            collapseBtn->setFixedSize(16, 16);
            collapseBtn->setArrowType(cap->isCollapsed() ? Qt::RightArrow : Qt::DownArrow);
            collapseBtn->setToolTip(cap->isCollapsed() ? "Expand" : "Collapse");
            header->addWidget(collapseBtn);
            header->addWidget(new QLabel("<b>" + cap->name() + "</b>"));
            header->addStretch();
            auto *removeBtn = new QToolButton;
            removeBtn->setAutoRaise(true);
            removeBtn->setFixedSize(16, 16);
            removeBtn->setIconSize(QSize(12, 12));
            removeBtn->setIcon(QIcon(":/resources/icons/trash.svg"));
            removeBtn->setToolTip("Remove capability");
            connect(removeBtn, &QToolButton::clicked, this, [this, state, cap](){
                state->removeCapability(cap);
                delete cap;
                if(m_node)
                    m_node->markStateEdited();
                rebuild();
            });
            header->addWidget(removeBtn);
            v->addLayout(header);

            // The channel rows live in their own widget so the whole block can
            // be folded away under the header.
            auto *body = new QWidget;
            auto *bodyLayout = new QVBoxLayout(body);
            bodyLayout->setContentsMargins(0, 0, 0, 0);
            bodyLayout->setSpacing(3);
            body->setVisible(!cap->isCollapsed());
            v->addWidget(body);

            connect(collapseBtn, &QToolButton::clicked, this, [cap, collapseBtn, body](){
                const bool collapsed = !cap->isCollapsed();
                cap->setCollapsed(collapsed);
                collapseBtn->setArrowType(collapsed ? Qt::RightArrow : Qt::DownArrow);
                collapseBtn->setToolTip(collapsed ? "Expand" : "Collapse");
                body->setVisible(!collapsed);
            });

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

                bodyLayout->addLayout(row);
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

    QMenu menu;
    for(const auto &entry : m_node->addableCapabilities())
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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QFrame>
#include "fixturestateeditor.h"
#include "graph/node/fixture/fixturestatenode.h"
#include "state/state.h"
#include "state/statecapability.h"
#include "sequence/channel.h"
#include "fixture/capability/fixturecapability.h"

namespace photon {

// Editor widget for a single capability channel, chosen by its value type.
static QWidget *makeChannelEditor(StateCapability *t_cap, int t_index, const ChannelInfo &t_info)
{
    switch(t_info.type)
    {
    case ChannelInfo::ChannelTypeColor:
    {
        auto *btn = new QPushButton;
        auto apply = [btn](const QColor &c){
            btn->setText(c.name());
            btn->setStyleSheet(QString("background:%1; color:%2;")
                .arg(c.name(), c.lightness() > 127 ? "black" : "white"));
        };
        apply(t_cap->getChannelValue(t_index).value<QColor>());
        QObject::connect(btn, &QPushButton::clicked, btn, [t_cap, t_index, apply](){
            const QColor c = QColorDialog::getColor(t_cap->getChannelValue(t_index).value<QColor>());
            if(c.isValid()) { t_cap->setChannelValue(t_index, c); apply(c); }
        });
        return btn;
    }
    case ChannelInfo::ChannelTypeBool:
    {
        auto *chk = new QCheckBox;
        chk->setChecked(t_cap->getChannelValue(t_index).toBool());
        QObject::connect(chk, &QCheckBox::toggled, chk, [t_cap, t_index](bool v){ t_cap->setChannelValue(t_index, v); });
        return chk;
    }
    case ChannelInfo::ChannelTypeInteger:
    case ChannelInfo::ChannelTypeIntegerStep:
    {
        auto *spin = new QSpinBox;
        spin->setRange(0, 255);
        spin->setValue(t_cap->getChannelValue(t_index).toInt());
        QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged), spin, [t_cap, t_index](int v){ t_cap->setChannelValue(t_index, v); });
        return spin;
    }
    default: // Number and anything else
    {
        auto *spin = new QDoubleSpinBox;
        spin->setRange(-10000.0, 10000.0);
        spin->setSingleStep(0.01);
        spin->setDecimals(3);
        spin->setValue(t_cap->getChannelValue(t_index).toDouble());
        QObject::connect(spin, qOverload<double>(&QDoubleSpinBox::valueChanged), spin, [t_cap, t_index](double v){ t_cap->setChannelValue(t_index, v); });
        return spin;
    }
    }
}

FixtureStateEditor::FixtureStateEditor(FixtureStateNode *t_node, QWidget *parent)
    : QWidget(parent), m_node(t_node)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    rebuild();
}

void FixtureStateEditor::rebuild()
{
    QLayoutItem *item;
    while((item = m_layout->takeAt(0)) != nullptr)
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
            v->setContentsMargins(4, 4, 4, 4);

            auto *header = new QHBoxLayout;
            header->addWidget(new QLabel("<b>" + cap->name() + "</b>"));
            header->addStretch();
            auto *removeBtn = new QPushButton("×");
            removeBtn->setMaximumWidth(24);
            connect(removeBtn, &QPushButton::clicked, this, [this, state, cap](){
                state->removeCapability(cap);
                delete cap;
                rebuild();
            });
            header->addWidget(removeBtn);
            v->addLayout(header);

            const auto channels = cap->availableChannels();
            for(int i = 0; i < channels.size(); ++i)
            {
                auto *row = new QHBoxLayout;
                row->addWidget(new QLabel(channels[i].name));

                auto *editor = makeChannelEditor(cap, i, channels[i]);
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

            m_layout->addWidget(frame);
        }
    }

    auto *addButton = new QPushButton("Add Capability");
    connect(addButton, &QPushButton::clicked, this, &FixtureStateEditor::openAddMenu);
    m_layout->addWidget(addButton);
}

void FixtureStateEditor::openAddMenu()
{
    State *state = m_node ? m_node->state() : nullptr;
    if(!state)
        return;

    struct Entry { const char *name; CapabilityType type; };
    static const Entry entries[] = {
        {"Dimmer", Capability_Dimmer},
        {"Color",  Capability_Color},
        {"Pan",    Capability_Pan},
        {"Tilt",   Capability_TiltAngleCentered},
        {"Strobe", Capability_Strobe},
        {"Focus",  Capability_Focus},
        {"Zoom",   Capability_Zoom},
    };

    QMenu menu;
    for(const auto &entry : entries)
    {
        const CapabilityType type = entry.type;
        menu.addAction(entry.name, this, [this, state, type](){
            state->addCapability(type);
            rebuild();
        });
    }
    menu.exec(QCursor::pos());
}

} // namespace photon

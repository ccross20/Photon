#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include "stateeditor.h"
#include "state/state.h"
#include "state/statecapability.h"
#include "gui/guimanager.h"
#include "sequence/stateclip.h"
#include "gui/color/colorwheelswatch.h"

namespace photon {

StateEditor::StateEditor(QWidget *parent)
    : QWidget{parent}
{
    m_layout = new QVBoxLayout;
    setLayout(m_layout);
}

void StateEditor::setClip(photon::StateClip* t_clip)
{
    m_clip = t_clip;
    selectState(m_clip->state());
}

void StateEditor::selectState(State *t_state)
{
    if(m_state)
    {
        disconnect(m_state, &State::capabilityAdded, this, &StateEditor::capabilityAdded);
        disconnect(m_state, &State::capabilityRemoved, this, &StateEditor::capabilityRemoved);
    }

    clearSelection();
    m_state = t_state;

    if(!m_state)
        return;

    connect(m_state, &State::capabilityAdded, this, &StateEditor::capabilityAdded);
    connect(m_state, &State::capabilityRemoved, this, &StateEditor::capabilityRemoved);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setColumnStretch(3,1);
    int row = 0;
    for(auto stateCapability : t_state->capabilities())
    {
        QLabel *label = new QLabel(stateCapability->name());


        QCheckBox *headerCheck = new QCheckBox;
        headerCheck->setChecked(stateCapability->isEnabled());
        connect(headerCheck, &QCheckBox::clicked,this,[stateCapability](bool value){
            stateCapability->setIsEnabled(value);
        });

        gridLayout->addWidget(headerCheck,row,0,stateCapability->availableChannels().length(),1,Qt::AlignHCenter | Qt::AlignTop);
        gridLayout->addWidget(label,row,1,stateCapability->availableChannels().length(),1,Qt::AlignLeft | Qt::AlignTop);


        int index = 0;
        for(const auto &channelInfo : stateCapability->availableChannels())
        {
            gridLayout->addWidget(new QLabel(channelInfo.name),row,2,Qt::AlignLeft | Qt::AlignTop);

            if(channelInfo.type == ChannelInfo::ChannelTypeNumber)
            {
                QDoubleSpinBox *spin = new QDoubleSpinBox;
                spin->setValue(stateCapability->getChannelValue(index).toDouble());
                spin->setMinimum(-2000.0);
                spin->setMaximum(2000.0);
                connect(spin, &QDoubleSpinBox::valueChanged, this, [stateCapability, index](double value){
                    stateCapability->setChannelValue(index, value);
                });
                gridLayout->addWidget(spin,row,3,Qt::AlignLeft | Qt::AlignTop);
            }
            else if(channelInfo.type == ChannelInfo::ChannelTypeBool)
            {
                QCheckBox *check = new QCheckBox;
                check->setChecked(stateCapability->getChannelValue(index).toBool());
                connect(check, &QCheckBox::clicked,this,[stateCapability, index](bool value){
                    stateCapability->setChannelValue(index, value);
                });
                gridLayout->addWidget(check,row,3,Qt::AlignLeft | Qt::AlignTop);
            }
            else if(channelInfo.type == ChannelInfo::ChannelTypeIntegerStep)
            {
                QComboBox *combo = new QComboBox;
                combo->insertItems(0, channelInfo.options);
                combo->setCurrentIndex(stateCapability->getChannelValue(index).toInt());
                connect(combo, &QComboBox::currentIndexChanged,this,[stateCapability, index](int value){
                    stateCapability->setChannelValue(index, value);
                });
                gridLayout->addWidget(combo,row,3,Qt::AlignLeft | Qt::AlignTop);
            }
            else if(channelInfo.type == ChannelInfo::ChannelTypeColor)
            {
                ColorWheelSwatch *swatch = new ColorWheelSwatch(stateCapability->getChannelValue(index).value<QColor>());

                connect(swatch, &ColorWheelSwatch::colorChanged,this,[stateCapability, index](QColor value){
                    stateCapability->setChannelValue(index, value);
                });
                gridLayout->addWidget(swatch,row,3,Qt::AlignLeft | Qt::AlignTop);
            }

            QPushButton *channelButton = new QPushButton("Channel");
            channelButton->setCheckable(true);
            channelButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
            connect(channelButton, &QPushButton::toggled,this,[this, stateCapability, index](bool value){
                if(value)
                    m_clip->addChannel(stateCapability->availableChannels().at(index));
            });
            gridLayout->addWidget(channelButton,row,4,Qt::AlignLeft | Qt::AlignTop);

            QPushButton *deleteButton = new QPushButton("Delete");
            deleteButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
            connect(deleteButton, &QPushButton::clicked,this,[t_state, stateCapability](){
                t_state->removeCapability(stateCapability);
            });
            gridLayout->addWidget(deleteButton,row,5,Qt::AlignLeft | Qt::AlignTop);

            ++index;
            ++row;
        }
    }

    m_addButton = new QPushButton("Add");
    m_addButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    connect(m_addButton, &QPushButton::clicked,this,&StateEditor::openAddMenu);
    gridLayout->addWidget(new QWidget{},row,0,1,4,Qt::AlignLeft | Qt::AlignTop);
    gridLayout->addWidget(m_addButton,row,5,Qt::AlignLeft | Qt::AlignTop);


    m_layout->addLayout(gridLayout);
    m_layout->addStretch();
}

void StateEditor::clearSelection()
{
    GuiManager::clearLayout(m_layout);
}

void StateEditor::openAddMenu()
{
    State *state = m_state;
    QMenu menu;
    menu.addAction("Strobe",[state](){state->addCapability(Capability_Strobe);});
    menu.addAction("Tilt",[state](){state->addCapability(Capability_Tilt);});
    menu.addAction("Tilt Angle",[state](){state->addCapability(Capability_TiltAngle);});
    menu.addAction("Tilt Angle Centered",[state](){state->addCapability(Capability_TiltAngleCentered);});
    menu.addAction("Dimmer",[state](){state->addCapability(Capability_Dimmer);});
    menu.addAction("Focus",[state](){state->addCapability(Capability_Focus);});
    menu.addAction("Zoom",[state](){state->addCapability(Capability_Zoom);});
    menu.addAction("Pan",[state](){state->addCapability(Capability_Pan);});
    menu.addAction("Color",[state](){state->addCapability(Capability_Color);});

    menu.exec(m_addButton->mapToGlobal(QPoint(0,0)));
}

void StateEditor::capabilityAdded(photon::StateCapability *t_capability)
{
    selectState(m_state);
}

void StateEditor::capabilityRemoved(photon::StateCapability *t_capability)
{
    selectState(m_state);
}

} // namespace photon

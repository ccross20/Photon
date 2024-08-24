#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include "sequence/viewer/stackedparameterwidget.h"
#include "beatintegereffect.h"
#include "sequence/channel.h"
#include "sequence/sequence.h"
#include "sequence/beatlayer.h"

namespace photon {

BeatIntegerEffectEditor::BeatIntegerEffectEditor(BeatIntegerEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    QSpinBox *minSpin = new QSpinBox;
    minSpin->setMinimum(-100000);
    minSpin->setMaximum(100000);
    minSpin->setValue(m_effect->minRange());
    connect(minSpin, &QSpinBox::valueChanged, this, &BeatIntegerEffectEditor::minRangeChanged);

    QSpinBox *maxSpin = new QSpinBox;
    maxSpin->setMinimum(-100000);
    maxSpin->setMaximum(100000);
    maxSpin->setValue(m_effect->maxRange());
    connect(maxSpin, &QSpinBox::valueChanged, this, &BeatIntegerEffectEditor::maxRangeChanged);

    QComboBox *modeCombo = new QComboBox;
    modeCombo->addItems(QStringList{"Increment","Decrement","Random"});
    connect(modeCombo, &QComboBox::currentIndexChanged, this, &BeatIntegerEffectEditor::modeChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(minSpin, "Minimum");
    paramWidget->addWidget(maxSpin, "Maximum");
    paramWidget->addWidget(modeCombo, "Mode");


    addWidget(paramWidget, "Beat Integer");
}

void BeatIntegerEffectEditor::minRangeChanged(int t_value)
{
    m_effect->setMinRange(t_value);
}
void BeatIntegerEffectEditor::maxRangeChanged(int t_value)
{
    m_effect->setMaxRange(t_value);
}
void BeatIntegerEffectEditor::modeChanged(int t_value)
{
    m_effect->setMode(static_cast<BeatIntegerEffect::BeatIntegerMode>(t_value));
}

BeatIntegerEffect::BeatIntegerEffect()
{

}

void BeatIntegerEffect::setMinRange(int t_value)
{
    m_min = t_value;
    rebuildCells();
    updated();
}

void BeatIntegerEffect::setMaxRange(int t_value)
{
    m_max = t_value;
    rebuildCells();
    updated();
}

void BeatIntegerEffect::setMode(BeatIntegerMode t_value)
{
    m_mode = t_value;
    rebuildCells();
    updated();
}

int BeatIntegerEffect::minRange() const
{
    return m_min;
}

int BeatIntegerEffect::maxRange() const
{
    return m_max;
}

BeatIntegerEffect::BeatIntegerMode BeatIntegerEffect::mode() const
{
    return m_mode;
}

void BeatIntegerEffect::addedToChannel()
{
    ChannelEffect::addedToChannel();

    rebuildCells();
}

float * BeatIntegerEffect::process(float *value, uint size, double t_time) const
{
    for(int i = 0; i < size; ++i)
    {
        if(m_cells.isEmpty())
        {
            value[i] = m_min;
            continue;
        }
        float globalTime = t_time + channel()->startTime();

        BeatCell t;
        t.time  = globalTime;

        auto it = std::lower_bound(m_cells.cbegin(), m_cells.cend(), t,[](const BeatCell &a, const BeatCell &b){return a.time < b.time;});

        value[i] = (*it).value;
    }

    return value;

}

void BeatIntegerEffect::rebuildCells()
{
    auto sequence = channel()->sequence();
    if(sequence)
    {
        m_cells.clear();

        if(sequence->beatLayers().isEmpty())
            return;

        auto layer = sequence->beatLayers().front();
        if(m_mode == ModeRandom)
        {
            QRandomGenerator generator(123);


            for(auto it = layer->beats().cbegin(); it != layer->beats().cend(); ++it)
            {
                BeatCell cell;

                cell.time = *it;
                cell.value = generator.bounded(m_min, m_max);
                m_cells.append(cell);
            }
        }
        else if(m_mode == ModeIncrement)
        {
            int val = m_min;
            for(auto it = layer->beats().cbegin(); it != layer->beats().cend(); ++it)
            {
                BeatCell cell;

                cell.time = *it;
                cell.value = val;
                m_cells.append(cell);

                ++val;
                if(val > m_max)
                    val = m_min;
            }
        }
        else
        {
            int val = m_max;
            for(auto it = layer->beats().cbegin(); it != layer->beats().cend(); ++it)
            {
                BeatCell cell;

                cell.time = *it;
                cell.value = val;
                m_cells.append(cell);

                --val;
                if(val < m_min)
                    val = m_max;
            }
        }


    }
}

ChannelEffectEditor *BeatIntegerEffect::createEditor()
{
    return new BeatIntegerEffectEditor(this);
}

void BeatIntegerEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_min = t_json.value("min").toInt(m_min);
    m_max = t_json.value("max").toInt(m_max);
    m_mode = static_cast<BeatIntegerEffect::BeatIntegerMode>(t_json.value("mode").toInt(m_mode));
}

void BeatIntegerEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("min", m_min);
    t_json.insert("max", m_max);
    t_json.insert("mode", m_mode);
}

EffectInformation BeatIntegerEffect::info()
{
    EffectInformation toReturn([](){return new BeatIntegerEffect;});
    toReturn.name = "Beat Integer";
    toReturn.effectId = "photon.effect.beat-integer";
    toReturn.categories.append("Generator");

    return toReturn;
}

} // namespace photon

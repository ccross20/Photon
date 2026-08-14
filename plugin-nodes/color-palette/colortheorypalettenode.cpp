#include <algorithm>
#include <cmath>
#include <QRandomGenerator>
#include "colortheorypalettenode.h"
#include "util/utils.h"

namespace photon {

namespace {

double wrapTurns(double t_h)
{
    t_h = std::fmod(t_h, 1.0);
    if(t_h < 0.0)
        t_h += 1.0;
    return t_h;
}

// Rescales an already-built color's saturation/value, applied uniformly as a
// last step regardless of which mode produced it (hue rotation or an RGB
// blend toward black/white/gray) - lets you push a whole palette brighter/
// muddier/more vivid without re-tuning every mode's own math.
QColor applyScale(const QColor &t_color, double t_satScale, double t_valScale)
{
    double h = t_color.hueF();
    double s = std::clamp(t_color.saturationF() * t_satScale, 0.0, 1.0);
    double v = std::clamp(t_color.valueF() * t_valScale, 0.0, 1.0);
    return QColor::fromHsvF(h < 0.0 ? 0.0 : h, s, v, t_color.alphaF());
}

} // namespace

keira::NodeInformation ColorTheoryPaletteNode::info()
{
    keira::NodeInformation toReturn([](){return new ColorTheoryPaletteNode;});
    toReturn.name = "Color Theory Palette";
    toReturn.nodeId = "photon.palette.color-theory";
    toReturn.categories = {"Color Palette"};

    return toReturn;
}

ColorTheoryPaletteNode::ColorTheoryPaletteNode() : keira::Node("photon.palette.color-theory") {}

void ColorTheoryPaletteNode::createParameters()
{
    m_colorParam = new ColorParameter("color", "Color", Qt::red);
    addParameter(m_colorParam);

    m_modeParam = new keira::OptionParameter("mode", "Mode", {
        "Complementary",
        "Split Complementary",
        "Analogous",
        "Triadic",
        "Tetradic (Square)",
        "Tetradic (Rectangle)",
        "Monochromatic",
        "Shades",
        "Tints",
        "Tones",
        "Compound",
        "Golden Angle",
        "Random Jitter",
    }, ModeComplementary);
    addParameter(m_modeParam);

    // Ignored by the fixed-relationship modes (Complementary/Triadic/both
    // Tetradics), which always produce their own fixed count of colors.
    m_countParam = new keira::IntegerParameter("count", "Count", 5);
    m_countParam->setMinimum(1);
    addParameter(m_countParam);

    // Dual meaning, documented per mode rather than split into two params -
    // this app's node parameters have no per-mode visibility, so one shared
    // field beats several that are only ever meaningful one at a time:
    //  - Hue-rotation modes (Split Complementary, Analogous, Tetradic
    //    Rectangle, Compound, Random Jitter): an angular offset/window, in
    //    turns (0..1 = 0..360deg).
    //  - Blend modes (Monochromatic, Shades, Tints, Tones): how far the
    //    ramp travels toward black/white/gray, as a fraction (0..1).
    //  - Unused by Triadic/both Tetradics/Golden Angle, whose spacing is
    //    fixed by the relationship itself.
    m_spreadParam = new keira::DecimalParameter("spread", "Spread", 1.0 / 6.0);
    m_spreadParam->setMinimum(0.0);
    m_spreadParam->setMaximum(1.0);
    addParameter(m_spreadParam);

    m_saturationParam = new keira::DecimalParameter("saturation", "Saturation Scale", 1.0);
    m_saturationParam->setMinimum(0.0);
    m_saturationParam->setMaximum(2.0);
    addParameter(m_saturationParam);

    m_brightnessParam = new keira::DecimalParameter("brightness", "Brightness Scale", 1.0);
    m_brightnessParam->setMinimum(0.0);
    m_brightnessParam->setMaximum(2.0);
    addParameter(m_brightnessParam);

    // Random Jitter only - deterministic per seed, so wiring in something
    // like DJ Connector's Song ID gives a palette that changes with the
    // track but lands on the same jitter again whenever that track repeats.
    m_seedParam = new keira::IntegerParameter("seed", "Seed", 1);
    addParameter(m_seedParam);

    m_paletteParam = new ColorPaletteParameter("palette", "Palette", ColorPalette{}, keira::AllowMultipleOutput);
    addParameter(m_paletteParam);
}

void ColorTheoryPaletteNode::evaluate(keira::EvaluationContext *) const
{
    QColor base = m_colorParam->value().value<QColor>();
    double baseH = base.hueF();
    if(baseH < 0.0)
        baseH = 0.0;
    double baseS = base.saturationF();
    double baseV = base.valueF();
    double baseA = base.alphaF();

    int mode = m_modeParam->value().toInt();
    int count = std::max(1, m_countParam->value().toInt());
    double spread = m_spreadParam->value().toDouble();
    double satScale = m_saturationParam->value().toDouble();
    double valScale = m_brightnessParam->value().toDouble();

    // Hue-rotation modes fill hueOffsets (turns, relative to the base hue);
    // blend modes fill colors directly (RGB interpolation toward black/
    // white/gray isn't a hue rotation at all). Exactly one of the two ends
    // up populated below.
    QVector<double> hueOffsets;
    QVector<QColor> colors;

    switch(mode)
    {
    case ModeComplementary:
        hueOffsets = {0.0, 0.5};
        break;

    case ModeSplitComplementary:
        hueOffsets = {0.0, 0.5 - spread, 0.5 + spread};
        break;

    case ModeAnalogous:
        for(int i = 0; i < count; ++i)
            hueOffsets << (count > 1 ? -spread / 2.0 + spread * i / double(count - 1) : 0.0);
        break;

    case ModeTriadic:
        hueOffsets = {0.0, 1.0 / 3.0, 2.0 / 3.0};
        break;

    case ModeTetradicSquare:
        hueOffsets = {0.0, 0.25, 0.5, 0.75};
        break;

    case ModeTetradicRectangle:
        hueOffsets = {0.0, spread, 0.5, 0.5 + spread};
        break;

    case ModeCompound:
    {
        // An analogous cluster (symmetric around the base) plus its exact
        // complement as a single accent - the cluster reads as one family,
        // the complement pops against it.
        int analogousCount = std::max(1, count - 1);
        for(int i = 0; i < analogousCount; ++i)
            hueOffsets << (analogousCount > 1 ? -spread / 2.0 + spread * i / double(analogousCount - 1) : 0.0);
        hueOffsets << 0.5;
        break;
    }

    case ModeGoldenAngle:
        // Each step is the golden angle (~137.5deg) around the wheel - the
        // standard trick for picking N maximally-spread-out hues for any N,
        // with no harmony to configure.
        for(int i = 0; i < count; ++i)
            hueOffsets << i * 0.6180339887498949;
        break;

    case ModeRandomJitter:
    {
        QRandomGenerator gen(static_cast<quint32>(m_seedParam->value().toInt()));
        hueOffsets << 0.0;   // the base color itself always leads the palette
        for(int i = 1; i < count; ++i)
            hueOffsets << (gen.generateDouble() * 2.0 - 1.0) * spread;
        break;
    }

    case ModeMonochromatic:
        // A single ramp from dark to light passing through the base color
        // at its midpoint, rather than only lightening (Tints) or only
        // darkening (Shades).
        for(int i = 0; i < count; ++i)
        {
            double t = count > 1 ? (-1.0 + 2.0 * i / double(count - 1)) * spread : 0.0;
            colors << (t < 0.0 ? blendColors(base, Qt::black, -t) : blendColors(base, Qt::white, t));
        }
        break;

    case ModeShades:
        for(int i = 0; i < count; ++i)
        {
            double t = count > 1 ? spread * i / double(count - 1) : 0.0;
            colors << blendColors(base, Qt::black, t);
        }
        break;

    case ModeTints:
        for(int i = 0; i < count; ++i)
        {
            double t = count > 1 ? spread * i / double(count - 1) : 0.0;
            colors << blendColors(base, Qt::white, t);
        }
        break;

    case ModeTones:
        for(int i = 0; i < count; ++i)
        {
            double t = count > 1 ? spread * i / double(count - 1) : 0.0;
            colors << blendColors(base, QColor(128, 128, 128), t);
        }
        break;
    }

    if(colors.isEmpty())
    {
        for(double offset : hueOffsets)
            colors << QColor::fromHsvF(wrapTurns(baseH + offset), baseS, baseV, baseA);
    }

    ColorPalette palette;
    palette.reserve(colors.size());
    for(const auto &c : colors)
        palette << applyScale(c, satScale, valScale);

    m_paletteParam->setValue(QVariant::fromValue(palette));
}

} // namespace photon

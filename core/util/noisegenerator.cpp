#include "noisegenerator.h"
#include "third-party/fastnoise.h"

namespace photon {

class NoiseGenerator::Impl
{
public:
    FastNoise xNoise;
    FastNoise yNoise;
    double amount = 1.0;
    QPointF offset;
    QPointF scale = QPointF{1.0,1.0};
    double max = 1.0;
    double min = -1.0;
    double delta = 2.0;
    NoiseMode mode;
};

NoiseGenerator::NoiseGenerator(NoiseMode mode):m_impl(new Impl())
{
    m_impl->mode = mode;
}

NoiseGenerator::~NoiseGenerator()
{
    delete m_impl;
}

void NoiseGenerator::setAmount(double amount)
{
    m_impl->amount = amount;
}

double NoiseGenerator::amount() const
{
    return m_impl->amount;
}

void NoiseGenerator::setOffset(QPointF offset)
{
    m_impl->offset = offset;
}

QPointF NoiseGenerator::offset() const
{
    return m_impl->offset;
}

void NoiseGenerator::setScale(QPointF scale)
{
    m_impl->scale = scale;
}

QPointF NoiseGenerator::scale() const
{
    return m_impl->scale;
}

void NoiseGenerator::setMax(double max)
{
    if(max < m_impl->min)
    {
        m_impl->max = m_impl->min;
        m_impl->min = max;
    }
    else
        m_impl->max = max;
    m_impl->delta = m_impl->max - m_impl->min;
}

double NoiseGenerator::max() const
{
    return m_impl->max;
}

void NoiseGenerator::setMin(double min)
{
    if(min > m_impl->max)
    {
        m_impl->min = m_impl->max;
        m_impl->max = min;
    }
    else
        m_impl->min = min;
    m_impl->delta = m_impl->max - m_impl->min;
}

void NoiseGenerator::setBounds(double min, double max)
{
    if(min > max)
        std::swap(min, max);
    m_impl->max = max;
    m_impl->min = min;
    m_impl->delta = m_impl->max - m_impl->min;
}

double NoiseGenerator::min() const
{
    return m_impl->min;
}

QPointF NoiseGenerator::boundedNoise(QPointF pt) const
{
    return pt + QPointF{((m_impl->xNoise.GetNoise((pt.x() - m_impl->offset.x()) * m_impl->scale.x(), (pt.y() - m_impl->offset.y()) * m_impl->scale.y())/2.0)*m_impl->delta)+m_impl->min,
                ((m_impl->yNoise.GetNoise((pt.x() - m_impl->offset.x()) * m_impl->scale.x(), (pt.y() - m_impl->offset.y()) * m_impl->scale.y())/2.0)/m_impl->delta)+m_impl->min};
}

QPointF NoiseGenerator::noise(QPointF pt) const
{
    return noise(pt.x(), pt.y());
}

QPointF NoiseGenerator::noise(QPointF pt, double amount) const
{
    return noise(pt.x(), pt.y(), amount);
}

QPointF NoiseGenerator::noise(double x, double y) const
{
    return noise(x,y, m_impl->amount);
}

QPointF NoiseGenerator::noise(double x, double y, double amount) const
{
    return QPointF{((((m_impl->xNoise.GetNoise((x - m_impl->offset.x()) * m_impl->scale.x(), (y - m_impl->offset.y()) * m_impl->scale.y()) + 1.0)/2.0) * m_impl->delta) + m_impl->min) * amount,
                ((((m_impl->yNoise.GetNoise((x - m_impl->offset.x()) * m_impl->scale.x(), (y - m_impl->offset.y()) * m_impl->scale.y()) + 1.0)/2.0) * m_impl->delta) + m_impl->min) * amount};
}

double NoiseGenerator::noise1D(QPointF pt) const
{
    return noise1D(pt.x(), pt.y());
}

double NoiseGenerator::noise1D(QPointF pt, double amount) const
{
    return noise1D(pt.x(), pt.y(), amount);
}

double NoiseGenerator::noise1D(double x, double y) const
{
    return noise1D(x, y, m_impl->amount);
}

double NoiseGenerator::noise1D(double x, double y, double amount) const
{
    return ((((m_impl->xNoise.GetNoise((x - m_impl->offset.x()) * m_impl->scale.x(), (y - m_impl->offset.y()) * m_impl->scale.y())+1.0)/2.0) * m_impl->delta) + m_impl->min) * amount;
}

void NoiseGenerator::setSeed(int seed)
{
    m_impl->xNoise.SetSeed(seed);
    m_impl->yNoise.SetSeed(seed+1);
}

int NoiseGenerator::seed() const
{
    return m_impl->xNoise.GetSeed();
}

void NoiseGenerator::setNoiseType(NoiseType type)
{
    m_impl->xNoise.SetNoiseType(static_cast<FastNoise::NoiseType>(type));
    m_impl->yNoise.SetNoiseType(static_cast<FastNoise::NoiseType>(type));
}

NoiseGenerator::NoiseType NoiseGenerator::noiseType() const
{
    return static_cast<NoiseType>(m_impl->xNoise.GetNoiseType());
}

void NoiseGenerator::setInterpolation(Interpolation inter)
{
    m_impl->xNoise.SetInterp(static_cast<FastNoise::Interp>(inter));
    m_impl->yNoise.SetInterp(static_cast<FastNoise::Interp>(inter));
}

NoiseGenerator::Interpolation NoiseGenerator::interpolation() const
{
    return static_cast<Interpolation>(m_impl->xNoise.GetInterp());
}

void NoiseGenerator::setFractalType(FractalType fractalType)
{
    m_impl->xNoise.SetFractalType(static_cast<FastNoise::FractalType>(fractalType));
    m_impl->yNoise.SetFractalType(static_cast<FastNoise::FractalType>(fractalType));
}

NoiseGenerator::FractalType NoiseGenerator::fractalType() const
{
    return static_cast<FractalType>(m_impl->xNoise.GetFractalType());
}

void NoiseGenerator::setCellularFunction(CellularDistanceFunction distance)
{
    m_impl->xNoise.SetCellularDistanceFunction(static_cast<FastNoise::CellularDistanceFunction>(distance));
    m_impl->yNoise.SetCellularDistanceFunction(static_cast<FastNoise::CellularDistanceFunction>(distance));
}

NoiseGenerator::CellularDistanceFunction NoiseGenerator::cellularFunction() const
{
    return static_cast<CellularDistanceFunction>(m_impl->xNoise.GetCellularDistanceFunction());
}

void NoiseGenerator::setCellularReturn(CellularReturnType cellularReturn)
{
    int value = cellularReturn;
    if(value > CellValue)
        value += 1;
    m_impl->xNoise.SetCellularReturnType(static_cast<FastNoise::CellularReturnType>(value));
    m_impl->yNoise.SetCellularReturnType(static_cast<FastNoise::CellularReturnType>(value));
}

NoiseGenerator::CellularReturnType NoiseGenerator::cellularReturn() const
{
    return static_cast<CellularReturnType>(m_impl->xNoise.GetCellularReturnType());
}

void NoiseGenerator::setFractalGain(double gain)
{
    m_impl->xNoise.SetFractalGain(gain);
    m_impl->yNoise.SetFractalGain(gain);
}

double NoiseGenerator::fractalGrain() const
{
    return m_impl->xNoise.GetFractalGain();
}

void NoiseGenerator::setFractalLacunarity(double value)
{
    m_impl->xNoise.SetFractalLacunarity(value);
    m_impl->yNoise.SetFractalLacunarity(value);
}

double NoiseGenerator::fractalLacunarity() const
{
    return m_impl->xNoise.GetFractalLacunarity();
}

void NoiseGenerator::setFractalOctaves(int octaves)
{
    m_impl->xNoise.SetFractalOctaves(octaves);
    m_impl->yNoise.SetFractalOctaves(octaves);
}

double NoiseGenerator::fractalOctaves() const
{
    return m_impl->xNoise.GetFractalOctaves();
}

void NoiseGenerator::setFrequency(double frequency)
{
    m_impl->xNoise.SetFrequency(frequency);
    m_impl->yNoise.SetFrequency(frequency);
}

double NoiseGenerator::frequency() const
{
    return m_impl->xNoise.GetFrequency();
}


} // namespace deco

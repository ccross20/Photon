#ifndef PHOTON_NOISEGENERATOR_H
#define PHOTON_NOISEGENERATOR_H
#include "photon-global.h"


namespace photon {

class PHOTONCORE_EXPORT NoiseGenerator
{
public:

    enum NoiseType { Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, Cellular, WhiteNoise, Cubic, CubicFractal };
    enum Interpolation { Linear, Hermite, Quintic };
    enum FractalType { FBM, Billow, RigidMulti };
    enum CellularDistanceFunction { Euclidean, Manhattan, Natural };
    enum CellularReturnType { CellValue, Distance, Distance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div };

    enum NoiseMode{
        NoiseMode1D,
        NoiseMode2D
    };

    NoiseGenerator(NoiseMode mode = NoiseMode2D);
    ~NoiseGenerator();

    void setAmount(double amount);
    double amount() const;
    void setOffset(QPointF offset);
    QPointF offset() const;
    void setScale(QPointF offset);
    QPointF scale() const;
    void setMax(double max);
    double max() const;
    void setMin(double min);
    double min() const;
    void setBounds(double min, double max);

    QPointF boundedNoise(QPointF pt) const;
    QPointF noise(QPointF pt) const;
    QPointF noise(QPointF pt, double amount) const;
    QPointF noise(double x, double y) const;
    QPointF noise(double x, double y, double amount) const;

    double noise1D(QPointF pt) const;
    double noise1D(QPointF pt, double amount) const;
    double noise1D(double x, double y) const;
    double noise1D(double x, double y, double amount) const;
    void setSeed(int seed);
    int seed() const;
    void setNoiseType(NoiseType type);
    NoiseType noiseType() const;
    void setInterpolation(Interpolation inter);
    Interpolation interpolation() const;
    void setFractalType(FractalType fractalType);
    FractalType fractalType() const;
    void setCellularFunction(CellularDistanceFunction distance);
    CellularDistanceFunction cellularFunction() const;
    void setCellularReturn(CellularReturnType cellularReturn);
    CellularReturnType cellularReturn() const;
    void setFractalGain(double gain);
    double fractalGrain() const;
    void setFractalLacunarity(double value);
    double fractalLacunarity() const;
    void setFractalOctaves(int octaves);
    double fractalOctaves() const;
    void setFrequency(double frequency);
    double frequency() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_NOISEGENERATOR_H

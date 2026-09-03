#include "mathconstantnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

const QByteArray MathConstantNode::ConstantInput = "constant";
const QByteArray MathConstantNode::Output = "output";

namespace {
// Spelled out as literals - this codebase is C++17, so <numbers> isn't
// available and M_PI isn't portable without _USE_MATH_DEFINES.
constexpr double kPi    = 3.14159265358979323846;
constexpr double kE     = 2.71828182845904523536;
constexpr double kSqrt2 = 1.41421356237309504880;
constexpr double kPhi   = 1.61803398874989484820;
constexpr double kLn2   = 0.69314718055994530942;
constexpr double kLn10  = 2.30258509299404568402;
}

class MathConstantNode::Impl
{
public:
    keira::OptionParameter *constantParam;
    keira::DecimalParameter *outputParam;
};

keira::NodeInformation MathConstantNode::info()
{
    keira::NodeInformation toReturn([](){return new MathConstantNode;});
    toReturn.name = "Math Constant";
    toReturn.nodeId = "photon.math.constant";
    toReturn.categories = {"Math"};

    return toReturn;
}

MathConstantNode::MathConstantNode() : keira::Node("photon.math.constant"), m_impl(new Impl)
{
    setName("Math Constant");
}

MathConstantNode::~MathConstantNode()
{
    delete m_impl;
}

void MathConstantNode::createParameters()
{
    // Appended, never reordered - the stored value is the option index.
    m_impl->constantParam = new keira::OptionParameter(ConstantInput, "Constant", {
        "π (Pi)",
        "τ (Tau = 2π)",
        "π/2",
        "e (Euler)",
        "√2",
        "φ (Golden Ratio)",
        "ln 2",
        "ln 10",
    }, Pi);
    addParameter(m_impl->constantParam);

    m_impl->outputParam = new keira::DecimalParameter(Output, "Output", kPi,
                                                      keira::AllowMultipleOutput);
    addParameter(m_impl->outputParam);
}

void MathConstantNode::evaluate(keira::EvaluationContext *) const
{
    double value = kPi;
    switch(m_impl->constantParam->value().toInt())
    {
    case Pi:          value = kPi;         break;
    case Tau:         value = 2.0 * kPi;   break;
    case HalfPi:      value = kPi / 2.0;   break;
    case E:           value = kE;          break;
    case Sqrt2:       value = kSqrt2;      break;
    case GoldenRatio: value = kPhi;        break;
    case Ln2:         value = kLn2;        break;
    case Ln10:        value = kLn10;       break;
    default:                               break;
    }

    m_impl->outputParam->setValue(value);
}

} // namespace photon

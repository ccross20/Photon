#include <QtMath>
#include <algorithm>
#include <cmath>
#include "ifnode.h"

namespace photon {

namespace {

// These inputs are doubles, so exact == would almost never fire: a value that
// has been through any arithmetic upstream lands a few ulps off the number the
// user typed. Comparing with a tolerance is what makes "A == B" behave the way
// it reads. The +1.0 on both sides is Qt's idiom for making qFuzzyCompare cope
// with zero (it is otherwise a purely relative comparison), and is the same
// form used elsewhere in this codebase.
bool nearlyEqual(double t_a, double t_b)
{
    return qFuzzyCompare(t_a + 1.0, t_b + 1.0);
}

// Whole-number test for the Even/Odd modes. Done entirely in doubles via fmod
// rather than casting to an integer: the inputs are unbounded, and converting
// one that doesn't fit an int is undefined (and trips Qt's checked conversion
// in debug). A is rounded first, so 4.0000001 counts as even.
bool isEvenValue(double t_value)
{
    const double whole = std::round(t_value);
    return std::fmod(std::fabs(whole), 2.0) < 0.5;
}

} // namespace

keira::NodeInformation IfNode::info()
{
    keira::NodeInformation toReturn([](){return new IfNode;});
    toReturn.name = "If";
    toReturn.nodeId = "photon.math.if";
    toReturn.categories = {"Math"};

    return toReturn;
}

IfNode::IfNode() : keira::Node("photon.math.if")
{
    setName("If");
}

void IfNode::createParameters()
{
    m_aParam = new keira::DecimalParameter("a", "A", 0.0);
    addParameter(m_aParam);

    m_bParam = new keira::DecimalParameter("b", "B", 0.0);
    addParameter(m_bParam);

    // Even and Odd ignore B. This app's node parameters have no per-mode
    // visibility, so which inputs matter is documented here rather than by
    // hiding the unused ones.
    //
    // Appended, never reordered: the stored value is the option's index, so
    // shuffling this list would silently change the mode of every saved node.
    m_modeParam = new keira::OptionParameter("mode", "Mode", {
        "A > B",
        "A < B",
        "A >= B",
        "A <= B",
        "A != B",
        "A == B",
        "A is Even",
        "A is Odd",
        "A is Divisible by B",
    }, ModeGreater);
    addParameter(m_modeParam);

    // Defaulting to 1 and 0 keeps the output reading as a plain boolean, while
    // still allowing any two numbers to be switched between.
    m_trueParam = new keira::DecimalParameter("trueValue", "True", 1.0);
    addParameter(m_trueParam);

    m_falseParam = new keira::DecimalParameter("falseValue", "False", 0.0);
    addParameter(m_falseParam);

    m_resultParam = new keira::DecimalParameter("result", "Result", 0.0, keira::AllowMultipleOutput);
    addParameter(m_resultParam);
}

void IfNode::evaluate(keira::EvaluationContext *) const
{
    const double a = m_aParam->value().toDouble();
    const double b = m_bParam->value().toDouble();

    bool result = false;

    switch(m_modeParam->value().toInt())
    {
    case ModeGreater:        result = a > b; break;
    case ModeLess:           result = a < b; break;
    // The two inclusive comparisons go through the same tolerance as ==, so
    // that a value which reads as equal doesn't fail >= on a rounding error.
    case ModeGreaterOrEqual: result = a > b || nearlyEqual(a, b); break;
    case ModeLessOrEqual:    result = a < b || nearlyEqual(a, b); break;
    case ModeNotEqual:       result = !nearlyEqual(a, b); break;
    case ModeEqual:          result = nearlyEqual(a, b); break;

    case ModeEven:           result = isEvenValue(a); break;
    case ModeOdd:            result = !isEvenValue(a); break;

    case ModeDivisible:
    {
        // Dividing by zero has no meaningful answer, so it reports false
        // rather than a NaN that would silently poison whatever this drives.
        if(nearlyEqual(b, 0.0))
        {
            result = false;
            break;
        }
        // A remainder of ~0 means B divides A exactly, which is the true case.
        // fmod lands just shy of either 0 or |b| when the division is exact but
        // the operands aren't exactly representable (fmod(0.3, 0.1) gives
        // ~0.0999..., not 0), so both ends count as divisible. The tolerance
        // scales with B so it stays meaningful for large divisors.
        const double remainder = std::fabs(std::fmod(a, b));
        const double divisor = std::fabs(b);
        const double tolerance = 1e-9 * std::max(1.0, divisor);
        result = remainder < tolerance || (divisor - remainder) < tolerance;
        break;
    }

    default: break;
    }

    m_resultParam->setValue(result ? m_trueParam->value().toDouble()
                                   : m_falseParam->value().toDouble());
}

} // namespace photon

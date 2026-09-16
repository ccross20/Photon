#include "colorpaletteinputnode.h"
#include "model/parameter/stringparameter.h"
#include "graph/parameter/colorpaletteparameter.h"

namespace photon {

const QByteArray ColorPaletteInputNode::Value = "value";
const QByteArray ColorPaletteInputNode::Name = "name";
const QByteArray ColorPaletteInputNode::Description = "description";

class ColorPaletteInputNode::Impl
{
public:
    ColorPaletteParameter *valueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
};


keira::NodeInformation ColorPaletteInputNode::info()
{
    keira::NodeInformation toReturn([](){return new ColorPaletteInputNode;});
    toReturn.name = "Color Palette Input";
    toReturn.nodeId = "photon.routine.color-palette-input";
    toReturn.categories = {"Input"};
    // Deliberately no "routine" - see the class comment.
    toReturn.graphs = QByteArrayList{"fixture","canvas","pixel"};
    toReturn.inputParameterType = ColorPaletteParameter::ParameterId;

    return toReturn;
}

ColorPaletteInputNode::ColorPaletteInputNode() : keira::GraphInputNode("photon.routine.color-palette-input"),m_impl(new Impl)
{
    setName("Color Palette Input");
    setValuePortId(Value);
}

ColorPaletteInputNode::~ColorPaletteInputNode()
{
    delete m_impl;
}

QString ColorPaletteInputNode::portName() const
{
    return m_impl->nameParam->value().toString();
}

void ColorPaletteInputNode::createParameters()
{
    m_impl->valueParam = new ColorPaletteParameter(Value,"Value", ColorPalette{}, keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "Palette", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
}

void ColorPaletteInputNode::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    keira::Node::setValue(t_id, t_value);

    // Renaming is the only edit here that needs to reach the outer mirror
    // parameter's display name - there's no Default/channelInfo to relay
    // (see the class comment).
    if(t_id == ColorPaletteInputNode::Name)
        notifyInterfaceChanged();
}

void ColorPaletteInputNode::evaluate(keira::EvaluationContext *) const
{
    // Subgraph path relies on the enclosing SubGraphNode's applyInputs() having
    // set the value port already; see NumberInputNode::evaluate.
}

} // namespace photon

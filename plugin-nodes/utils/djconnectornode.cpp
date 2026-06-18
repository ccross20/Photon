#include "djconnectornode.h"
#include "virtualdj/virtualdjconnector.h"
#include "photoncore.h"

namespace photon {


keira::NodeInformation DJConnectorNode::info()
{
    keira::NodeInformation toReturn([](){return new DJConnectorNode;});
    toReturn.name = "DJ Connector";
    toReturn.nodeId = "photon.utils.dj_connector";
    toReturn.categories = {"Utilities"};

    return toReturn;
}

DJConnectorNode::DJConnectorNode() : keira::Node("photon.utils.dj_connector") {}


void DJConnectorNode::createParameters()
{
    bpmParam = new keira::DecimalParameter("bpm","BPM", 0.0);
    addParameter(bpmParam);

    beatParam = new keira::IntegerParameter("beat","Beat",0);
    addParameter(beatParam);

    beatProgressParam = new keira::DecimalParameter("beatProgress","Beat Progress", 0.0);
    addParameter(beatProgressParam);

    beatProgress2Param = new keira::DecimalParameter("beatProgress2","Beat Progress x2", 0.0);
    addParameter(beatProgress2Param);

    beatProgress4Param = new keira::DecimalParameter("beatProgress4","Beat Progress x4", 0.0);
    addParameter(beatProgress4Param);

    beatIntensityParam = new keira::DecimalParameter("beatIntensity","Beat Intensity", 0.0);
    addParameter(beatIntensityParam);

    beatAmountParam = new keira::DecimalParameter("beatAmount","Beat Amount", 0.0);
    addParameter(beatAmountParam);

}

void DJConnectorNode::evaluate(keira::EvaluationContext *t_context) const
{
    bpmParam->setValue(photonApp->djConnector()->bpm);
    beatParam->setValue(photonApp->djConnector()->beatNumber);
    beatProgressParam->setValue(photonApp->djConnector()->beatProgress);
    beatProgress2Param->setValue(photonApp->djConnector()->beatProgress2);
    beatProgress4Param->setValue(photonApp->djConnector()->beatProgress4);
    beatIntensityParam->setValue(photonApp->djConnector()->beatIntensity);
    beatAmountParam->setValue(photonApp->djConnector()->beatAmount);

}

} // namespace photon

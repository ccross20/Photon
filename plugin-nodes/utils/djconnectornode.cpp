#include "djconnectornode.h"
#include "virtualdj/virtualdjconnector.h"
#include "photoncore.h"

namespace photon {

namespace {

// Folds a track's identity string into a stable, pseudo-unique integer.
//
// Deliberately a hand-rolled FNV-1a rather than qHash(): Qt randomizes its
// string hash seed per process (and reserves the right to change the algorithm
// between versions), so qHash would hand the same song a different number on
// every launch. The whole point here is the opposite - one song maps to one
// value, always - so that a show seeded from this looks the same every time
// that track comes up.
int songIdFromIdentity(const QString &t_identity)
{
    if(t_identity.isEmpty())
        return 0;

    const QByteArray utf8 = t_identity.toUtf8();
    quint32 hash = 2166136261u;                 // FNV-1a 32-bit offset basis
    for(char c : utf8)
    {
        hash ^= static_cast<quint8>(c);
        hash *= 16777619u;                      // FNV-1a 32-bit prime
    }

    // Clamp to a positive int: downstream seeds read this back with
    // value().toInt(), and some use it for modulo/indexing where a negative
    // would misbehave.
    return static_cast<int>(hash & 0x7fffffffu);
}

} // namespace

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
    bpmParam = new keira::DecimalParameter("bpm","BPM", 0.0, keira::AllowMultipleOutput);
    addParameter(bpmParam);

    beatParam = new keira::IntegerParameter("beat","Beat",0, keira::AllowMultipleOutput);
    addParameter(beatParam);

    beatProgressParam = new keira::DecimalParameter("beatProgress","Beat Progress", 0.0, keira::AllowMultipleOutput);
    addParameter(beatProgressParam);

    beatProgress2Param = new keira::DecimalParameter("beatProgress2","Beat Progress x2", 0.0, keira::AllowMultipleOutput);
    addParameter(beatProgress2Param);

    beatProgress4Param = new keira::DecimalParameter("beatProgress4","Beat Progress x4", 0.0, keira::AllowMultipleOutput);
    addParameter(beatProgress4Param);

    beatIntensityParam = new keira::DecimalParameter("beatIntensity","Beat Intensity", 0.0, keira::AllowMultipleOutput);
    addParameter(beatIntensityParam);

    beatAmountParam = new keira::DecimalParameter("beatAmount","Beat Amount", 0.0, keira::AllowMultipleOutput);
    addParameter(beatAmountParam);

    // Pseudo-unique per track, for wiring into seed inputs so the graph
    // re-rolls its random values when the song changes (and lands on the same
    // values again whenever that same song comes back).
    songIdParam = new keira::IntegerParameter("songId","Song ID", 0, keira::AllowMultipleOutput);
    addParameter(songIdParam);
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

    // path is what the connector itself treats as the track-change signal, so
    // it's the identity to key on; title+artist is the fallback for a source
    // that reports metadata but no file (a stream, say). The newline keeps
    // "AB"+"C" from colliding with "A"+"BC".
    const QString identity = !photonApp->djConnector()->path.isEmpty()
                                 ? photonApp->djConnector()->path
                                 : photonApp->djConnector()->title + '\n' + photonApp->djConnector()->artist;

    if(identity != m_lastSongIdentity)
    {
        m_lastSongIdentity = identity;
        m_songId = songIdFromIdentity(identity);
    }
    songIdParam->setValue(m_songId);
}

} // namespace photon

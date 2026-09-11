#include <cmath>
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

    // Index order must track kRateMultipliers in evaluate().
    beatRateParam = new keira::OptionParameter("beatRate", "Rate", {"/4", "/2", "1", "x2", "x4", "x8"}, 2);
    addParameter(beatRateParam);

    beatOffsetParam = new keira::DecimalParameter("beatOffset", "Offset", 0.0);
    addParameter(beatOffsetParam);

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

    // Beat reducer: re-grid the continuous beat position onto a slower or
    // faster pulse before splitting it back into a whole-beat count + 0..1
    // progress, the same shape "Beat"/"Beat Progress" always had - so "1"
    // (the default) reproduces the raw DJ beat exactly, and downstream
    // graphs built against these two outputs keep working whatever rate is
    // picked. Order must track the option list in createParameters().
    static const double kRateMultipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
    const int rateIndex = qBound(0, beatRateParam->value().toInt(), 5);
    const double rate = kRateMultipliers[rateIndex];
    const double offset = beatOffsetParam->value().toDouble();

    const double rawBeatPosition = double(photonApp->djConnector()->beatNumber)
        + photonApp->djConnector()->beatProgress;
    const double scaledPosition = (rawBeatPosition - offset) * rate;
    const double reducedBeat = std::floor(scaledPosition);

    beatParam->setValue(int(reducedBeat));
    beatProgressParam->setValue(scaledPosition - reducedBeat);

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

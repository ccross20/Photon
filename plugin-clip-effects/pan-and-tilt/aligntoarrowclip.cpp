
#include <QMatrix4x4>
#include "aligntoarrowclip.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"
#include "sequence/fixtureclip.h"
#include "project/project.h"
#include "scene/sceneiterator.h"
#include "photoncore.h"
#include "channel/parameter/numberchannelparameter.h"

namespace photon {

AlignToArrowClip::AlignToArrowClip()
{

}

void AlignToArrowClip::init()
{
    ClipEffect::init();
    addChannelParameter(new NumberChannelParameter("tilt"));
    addChannelParameter(new NumberChannelParameter("pan"));
    auto arrowObj = SceneIterator::FindOne(photonApp->project()->sceneRoot(),[](SceneObject *obj){
        return dynamic_cast<SceneArrow*>(obj);
    });

    m_arrow = static_cast<SceneArrow*>(arrowObj);
}

void AlignToArrowClip::evaluate(FixtureClipEffectEvaluationContext &t_context)
{
    QMatrix4x4 matrix;
    //matrix.translate(fixture->globalPosition());
    //matrix.rotate(QQuaternion::fromEulerAngles(fixture->rotation()));

    auto fixture = t_context.fixture;

    if(!m_arrow)
    {
        qDebug() << "No arrow";
        return;
    }

    float panOffset = t_context.channelValues["pan"].toDouble();
    float tiltOffset = t_context.channelValues["tilt"].toDouble();

    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    initialRelativeTime -= fixtureClip()->falloff(fixture);

    auto arrowMatrix = m_arrow->globalMatrix();

    arrowMatrix.rotate(panOffset,1,0,0);
    arrowMatrix.rotate(tiltOffset,0,1,0);
    //arrowMatrix.rotate(QQuaternion::fromEulerAngles(QVector3D{panOffset, tiltOffset,0.0}));
    auto vec = arrowMatrix.mapVector(QVector3D{0,0,1});

    auto pt3D = fixture->globalPosition() + vec;

    matrix = fixture->globalMatrix();



    auto panRad = matrix.inverted().map(pt3D).normalized();
    float panDeg = qRadiansToDegrees(atan2(panRad.x(),panRad.z()));

/*
    if(panRad.y() < 0.0)
        panDeg += 360;
    if(panRad.z() < 0.0 && panRad.y() > 0.0)
        panDeg += 180;
*/

    //qDebug() << panDeg << panRad.x() << panRad.z();
/*
    if(panDeg > 90)
        panDeg -= 180;
    else if(panDeg < -90)
        panDeg += 180;
*/

    auto pans = fixture->findCapability(Capability_Pan);
    if(!pans.isEmpty())
        static_cast<AngleCapability*>(pans.front())->setAngleDegreesCentered(panDeg, t_context.dmxMatrix, clip()->strengthAtTime(initialRelativeTime));

    QMatrix4x4 tiltMat;
    tiltMat.translate(0,.4,0);
    tiltMat.rotate(QQuaternion::fromEulerAngles(QVector3D{0,panDeg,90}));

    matrix = matrix * tiltMat;

    auto tiltRad = matrix.inverted().map(pt3D);
    float tiltDeg = qRadiansToDegrees(atan2(tiltRad.z(),tiltRad.x()));

    //qDebug() << tiltDeg;

    auto tilts = fixture->findCapability(Capability_Tilt);
    if(!tilts.isEmpty())
        static_cast<AngleCapability*>(tilts.front())->setAngleDegreesCentered(tiltDeg, t_context.dmxMatrix, clip()->strengthAtTime(initialRelativeTime));

}

ClipEffectInformation AlignToArrowClip::info()
{
    ClipEffectInformation toReturn([](){return new AlignToArrowClip;});
    toReturn.name = "Align to Arrow";
    toReturn.id = "photon.clip.arrow-align";
    toReturn.categories.append("Position");

    return toReturn;
}

} // namespace photon

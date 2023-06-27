#include <QPainterPath>
#include <QMatrix4x4>
#include <QTransform>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "tracepathclip.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"
#include "sequence/clip.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"

namespace photon {

TracePathWidget::TracePathWidget(TracePathClip *t_effect):QWidget(),m_effect(t_effect)
{
    QFormLayout *formLayout = new QFormLayout;
    auto radiusSpin = new QDoubleSpinBox();
    radiusSpin->setValue(m_effect->radius);
    radiusSpin->setMinimum(0.0);
    radiusSpin->setMaximum(100.0);
    connect(radiusSpin, &QDoubleSpinBox::valueChanged, this, &TracePathWidget::updateRadius);

    formLayout->addRow("Radius", radiusSpin);

    auto offsetSpin = new QDoubleSpinBox();
    offsetSpin->setValue(m_effect->offset);
    offsetSpin->setMinimum(-10.0);
    offsetSpin->setMaximum(10.0);
    connect(offsetSpin, &QDoubleSpinBox::valueChanged, this, &TracePathWidget::updateOffset);

    formLayout->addRow("Offset", offsetSpin);

    auto button = new QPushButton("Assign");
    connect(button, &QPushButton::clicked,this,[this](){m_effect->autoAssignFixtures(0.0);});
    formLayout->addRow("Assign Fixtures", button);



    setLayout(formLayout);

}

void TracePathWidget::updateRadius(double t_radius)
{
    m_effect->radius = t_radius;
    m_effect->updated();
}

void TracePathWidget::updateOffset(double t_offset)
{
    m_effect->offset = t_offset;
    m_effect->updated();
}


TracePathClip::TracePathClip()
{

}

void TracePathClip::init()
{
    ClipEffect::init();
    addChannel(ChannelInfo{"radius",ChannelInfo::ChannelTypeNumber,"Radius","Size of the shape",1.0});
    addChannel(ChannelInfo{"offset",ChannelInfo::ChannelTypeNumber,"Offset","Offset from start",0.0});
}

QWidget *TracePathClip::createEditor()
{
    return new TracePathWidget(this);
}

void TracePathClip::autoAssignFixtures(double t_time)
{
    fixtures.clear();
    auto maskedFixtures = clip()->maskedFixtures();

    double segLength = 1.0/maskedFixtures.length();
    double currentLength = 0.0;

    auto path = generatePath(t_time);

    struct DistanceData
    {
        DistanceData(Fixture *t_fixture, double t_distance, int t_index):distance(t_distance),fixture(t_fixture),index(t_index) {}
        double distance;
        Fixture *fixture;
        int index;
    };

    std::vector<DistanceData> sortedVector;


    for(int i = 0; i < maskedFixtures.length(); ++i)
    {
        auto pt = path.pointAtPercent(currentLength);

        QVector3D pt3D{static_cast<float>(pt.x()),0.0f, static_cast<float>(pt.y())};

        std::vector<DistanceData> vec;
        for(auto fixture : maskedFixtures)
        {
            sortedVector.push_back(DistanceData(fixture, fixture->globalPosition().distanceToPoint(pt3D),i));
        }



        currentLength += segLength;
    }

    std::sort(sortedVector.begin(), sortedVector.end(),[](const DistanceData &a, const DistanceData &b){
        return a.distance < b.distance;
    });

    std::vector<bool> takenIndices;
    takenIndices.resize(maskedFixtures.length(), false);
    fixtures.resize(maskedFixtures.length());

    for(auto fixture : maskedFixtures)
    {
        for(const auto &ddata : sortedVector)
        {
            if(ddata.fixture == fixture && !takenIndices[ddata.index])
            {
                takenIndices[ddata.index] = true;
                fixtures[ddata.index] = fixture;
                break;
            }
        }
    }
}

QPainterPath TracePathClip::generatePath(double t_time) const
{
    QPainterPath path;
    double radius = getNumberAtTime("radius", t_time);
    path.addEllipse(-radius,-radius,radius *2.0, radius * 2.0);
    return path;
}

void TracePathClip::evaluateFixture(ClipEffectEvaluationContext &t_context) const
{
    int currentIndex = 0;
    for(int i = 0; i < fixtures.length(); ++i)
    {
        if(fixtures[i] == t_context.fixture)
        {
            currentIndex = i;
            break;
        }
    }



    QPainterPath path;
    double off = t_context.channelValues["offset"].toDouble();
    double rad = t_context.channelValues["radius"].toDouble();
    path.addEllipse(-rad,-rad,rad *2.0, rad * 2.0);

    double segLength = 1.0/t_context.fixtureTotal;
    double currentLength = segLength * currentIndex;

    auto fixture = t_context.fixture;

    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    currentLength += off;

    if(currentLength > 1.0)
        currentLength -= std::floor(currentLength);
    else if(currentLength < -1.0)
        currentLength += std::floor(currentLength);

    if(currentLength < 0.0)
        currentLength = 1.0 - currentLength;

    initialRelativeTime -= clip()->falloff(fixture);
    auto pt = path.pointAtPercent(currentLength);
    QVector3D pt3D{static_cast<float>(pt.x()),0.0f, static_cast<float>(pt.y())};

    QMatrix4x4 matrix;
    matrix.translate(fixture->globalPosition());
    matrix.rotate(QQuaternion::fromEulerAngles(fixture->rotation()));


    auto panRad = matrix.inverted().map(pt3D);
    float panDeg = qRadiansToDegrees(atan2(panRad.x(),panRad.z()));

    //qDebug() << panDeg;

    if(panDeg > 90)
        panDeg -= 180;
    else if(panDeg < -90)
        panDeg += 180;

    auto pans = fixture->findCapability(Capability_Pan);
    if(!pans.isEmpty())
        static_cast<AngleCapability*>(pans.front())->setAngleDegreesCentered(panDeg, t_context.dmxMatrix, clip()->strengthAtTime(initialRelativeTime));

    QMatrix4x4 tiltMat;
    tiltMat.translate(0,.4,0);
    tiltMat.rotate(QQuaternion::fromEulerAngles(QVector3D{0,panDeg,90}));

    matrix = matrix * tiltMat;

    auto tiltRad = matrix.inverted().map(pt3D);
    float tiltDeg = qRadiansToDegrees(atan2(tiltRad.z(),tiltRad.x()));

    auto tilts = fixture->findCapability(Capability_Tilt);
    if(!tilts.isEmpty())
        static_cast<AngleCapability*>(tilts.front())->setAngleDegreesCentered(tiltDeg, t_context.dmxMatrix, clip()->strengthAtTime(initialRelativeTime));

}


void TracePathClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    ClipEffect::readFromJson(t_json, t_context);

    fixtures.clear();
    auto fixtureArray = t_json.value("fixtures").toArray();

    for(auto fixObj : fixtureArray)
    {
        auto fix = FixtureCollection::fixtureById(t_context.project, fixObj.toString().toLatin1());
        if(fix)
            fixtures.append(fix);
    }
}

void TracePathClip::writeToJson(QJsonObject &t_json) const
{
    ClipEffect::writeToJson(t_json);

    QJsonArray fixtureArray;

    for(auto fix : fixtures)
        fixtureArray.append(QString{fix->uniqueId()});

    t_json.insert("fixtures", fixtureArray);
}


ClipEffectInformation TracePathClip::info()
{
    ClipEffectInformation toReturn([](){return new TracePathClip;});
    toReturn.name = "Trace Path";
    toReturn.id = "photon.clip.trace-path";
    toReturn.categories.append("Position");

    return toReturn;
}

} // namespace photon

#include "sequencetest.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixturemask.h"
#include "fixture/fixture.h"
#include "sequence/sequence.h"
#include "sequence/layer.h"
#include "sequence/clip.h"
#include "routine/routine.h"

namespace photon {

SequenceTest::SequenceTest(QObject *parent)
    : QObject{parent}
{

}

void SequenceTest::simpleProcess()
{
    /*
    Project *proj = new Project;

    Fixture *fixture = new Fixture;

    Sequence *sequence = new Sequence;
    proj->addSequence(sequence);

    Layer *layer = new Layer;

    Clip *clip = new Clip;
    clip->setDuration(25);

    //clip->setMask(mask);

    Routine *routine = new Routine;

    proj->fixtures()->addFixture(fixture);

    sequence->addLayer(layer);
    layer->addClip(clip);
    clip->setRoutine(routine);


    delete clip;
    delete sequence;

    delete proj;
    */
}

} // namespace photon

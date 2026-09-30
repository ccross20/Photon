#include <QTest>
#include "sequencetest.h"
#include "audio/songdata.h"
#include <QAbstractItemModelTester>
#include "sequence/viewer/clipmodel.h"
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

void SequenceTest::beatPulseTimes()
{
    // Uneven spacing after the 4th beat, so interpolation is exercised.
    BeatGrid grid;
    grid.setBeats({0.0, 0.5, 1.0, 1.5, 2.1, 2.7, 3.3, 3.9});

    QCOMPARE(grid.pulseTimes(1.0, 0.0), grid.beats());

    const QVector<double> halves = grid.pulseTimes(2.0, 0.0);
    QCOMPARE(halves.size(), 15);
    QCOMPARE(halves[1], 0.25);
    QCOMPARE(halves[9], 2.4);
    QCOMPARE(halves.last(), 3.9);

    QCOMPARE(grid.pulseTimes(0.25, 0.0), (QVector<double>{0.0, 2.1}));

    const QVector<double> shifted = grid.pulseTimes(1.0, 0.5);
    QCOMPARE(shifted.size(), 7);
    QCOMPARE(shifted.first(), 0.25);

    // First analysed beat is beat 2 of its bar: bars start at indices 3 and 7.
    grid.setBarStartOffset(1);
    QCOMPARE(grid.pulseTimes(0.25, 0.0), (QVector<double>{1.5, 3.9}));
}

void SequenceTest::clipModelStaysConsistent()
{
    // Rows added/removed deep in the tree must be reported under the right
    // parent - the view keeps its expanded state (and selection) by that.
    ClipModel model;
    QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

    auto *top = new FolderData("Clip", AbstractTreeData::DataChannel, false);
    model.root()->addChild(top);
    auto *channels = new FolderData("Channels", AbstractTreeData::DataChannel, false);
    top->addChild(channels);
    auto *channel = new FolderData("Channel", AbstractTreeData::DataChannel, true);   // holds an "Add..." row
    channels->addChild(channel);

    const QModelIndex channelsIndex = model.indexForData(channels);
    QVERIFY(channelsIndex.isValid());
    QCOMPARE(model.parent(model.indexForData(top)), QModelIndex());
    QCOMPARE(model.parent(channelsIndex), model.indexForData(top));

    // An effect-like row inserted before the channel's "Add..." row.
    auto *effect = new FolderData("Effect", AbstractTreeData::DataChannel, false);
    channel->insertChild(effect, channel->childCount() - 1);
    QCOMPARE(model.indexForData(effect).parent(), model.indexForData(channel));
    QCOMPARE(model.indexForData(effect).row(), 0);

    channel->removeChild(effect);
    QCOMPARE(model.rowCount(model.indexForData(channel)), 1);
}

} // namespace photon

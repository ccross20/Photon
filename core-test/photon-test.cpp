#include <QMap>
#include <QDebug>
#include <QApplication>
#include <QTest>
#include <memory>
#include <map>
#include "photon-global.h"
#include "bus/bustest.h"
#include "fixture/fixturecollectiontest.h"
#include "fixture/fixturecapabilitytest.h"
#include "fixture/colorcalibrationtest.h"
#include "fixture/lasertest.h"
#include "virtualdj/virtualdjplaybacktest.h"
#include "sequence/sequencetest.h"
#include "scene/sceneobjecttest.h"
#include "project/surfaceownershiptest.h"
#include "project/projectresourcetest.h"
#include "project/projectmodeltest.h"
#include "project/tagtest.h"
#include "gui/propertyaddresstest.h"


int main(int argc, char *argv[])
{
    //QApplication app(argc, argv);


    qSetMessagePattern("%{function} [%{line}] %{message}");



    //exo::ExoCore core(argc, argv);


    // Built from argv rather than QCoreApplication::arguments(): there is no
    // QCoreApplication instance here, so that call returns an empty list and
    // qExec() silently runs nothing.
    QStringList arguments;
    for(int i = 0; i < argc; ++i)
        arguments << QString::fromLocal8Bit(argv[i]);

    std::map<QString, std::unique_ptr<QObject>> tests;

    tests.emplace("bus_test", std::make_unique<photon::BusTest>());
    tests.emplace("property_address_test", std::make_unique<photon::PropertyAddressTest>());
    tests.emplace("fixture_collection_test", std::make_unique<photon::FixtureCollectionTest>());
    tests.emplace("fixture_capability_test", std::make_unique<photon::FixtureCapabilityTest>());
    tests.emplace("color_calibration_test", std::make_unique<photon::ColorCalibrationTest>());
    tests.emplace("laser_test", std::make_unique<photon::LaserTest>());
    tests.emplace("virtualdj_playback_test", std::make_unique<photon::VirtualDJPlaybackTest>());
    tests.emplace("sequence_test", std::make_unique<photon::SequenceTest>());
    tests.emplace("scene_object_test", std::make_unique<photon::SceneObjectTest>());
    tests.emplace("surface_ownership_test", std::make_unique<photon::SurfaceOwnershipTest>());
    tests.emplace("project_resource_test", std::make_unique<photon::ProjectResourceTest>());
    tests.emplace("project_model_test", std::make_unique<photon::ProjectModelTest>());
    tests.emplace("tag_test", std::make_unique<photon::TagTest>());

    if (arguments.size() >= 3 && arguments[1] == "-select") {
        QString testName = arguments[2];
        auto iter = tests.begin();
        while(iter != tests.end()) {
            if (iter->first != testName) {
                iter = tests.erase(iter);
            } else {
                ++iter;
            }
        }
        arguments.removeOne("-select");
        arguments.removeOne(testName);
    }

    int status = 0;
    for(auto& test : tests) {
        status |= QTest::qExec(test.second.get(), arguments);
    }

    return status;
}

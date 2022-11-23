#include <QMap>
#include <QDebug>
#include <QApplication>
#include <QTest>
#include <memory>
#include <map>
#include "keira-global.h"
#include "evaluationtest.h"


int main(int argc, char *argv[])
{
    //QApplication app(argc, argv);


    qSetMessagePattern("%{function} [%{line}] %{message}");



    //exo::ExoCore core(argc, argv);


    QStringList arguments = QCoreApplication::arguments();

    std::map<QString, std::unique_ptr<QObject>> tests;

    tests.emplace("fixture_collection_test", std::make_unique<keira::EvaluationTest>());

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

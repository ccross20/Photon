#ifndef PHOTON_PROJECTRESOURCETEST_H
#define PHOTON_PROJECTRESOURCETEST_H

#include <QObject>

namespace photon {

// Covers the shared ProjectResource contract: tag storage/normalisation, the
// notifier, json round-tripping, and the per-type adoptions.
class ProjectResourceTest : public QObject
{
    Q_OBJECT
public:
    explicit ProjectResourceTest(QObject *parent = nullptr);

private slots:
    void tagsAreNormalisedAndSorted();
    void unchangedTagsDoNotNotify();
    void tagsRoundTripThroughJson();
    void sceneObjectBridgesBothNotificationPaths();
    void sceneObjectTagAliasesShareStorage();
    void fixtureGroupIsAnIdentifiedResource();
    void fixtureGroupQueryChangeReachesCollection();
    void sequenceIdRoundTrips();

    void selectionCarriesAnyResourceType();
    void sceneConsumersSeeOnlyTheirSlice();
    void sceneSelectionSignalsStayQuietWhenTheSliceIsUnchanged();
};

} // namespace photon

#endif // PHOTON_PROJECTRESOURCETEST_H

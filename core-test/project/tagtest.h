#ifndef PHOTON_TAGTEST_H
#define PHOTON_TAGTEST_H

#include <QObject>

namespace photon {

// Covers the non-interactive parts of the tag UX overhaul: deterministic
// chip coloring and Project::allTags()'s live scan across every resource
// type. The chip/picker widgets themselves are interactive and covered by
// manual verification instead, consistent with this project's existing test
// scope.
class TagTest : public QObject
{
    Q_OBJECT
public:
    explicit TagTest(QObject *parent = nullptr);

private slots:
    void tagColorIsDeterministic();
    void tagColorIsCaseInsensitive();
    void tagTextColorContrastsBackground();
    void allTagsUnionsEveryResourceType();
    void allTagsIsSortedAndDeduped();
};

} // namespace photon

#endif // PHOTON_TAGTEST_H

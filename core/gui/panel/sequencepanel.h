#ifndef PHOTON_SEQUENCEPANEL_H
#define PHOTON_SEQUENCEPANEL_H

#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class SequencePanel : public Panel
{
    Q_OBJECT
public:
    SequencePanel();
    ~SequencePanel();
    void setSequence(Sequence *);
    Sequence *sequence() const;

    void processPreview(ProcessContext &context);

protected:

    void didDeactivate() override;
    void didActivate() override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SEQUENCEPANEL_H

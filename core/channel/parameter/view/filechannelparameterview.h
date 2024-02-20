#ifndef PHOTON_FILECHANNELPARAMETERVIEW_H
#define PHOTON_FILECHANNELPARAMETERVIEW_H

#include "channelparameterview.h"

namespace photon {

class FileParameter;

class PHOTONCORE_EXPORT FileChannelParameterView : public ChannelParameterView
{
    Q_OBJECT
public:
    FileChannelParameterView(FileParameter *);
    ~FileChannelParameterView();

private slots:
    void clickBrowse();
    void pathUpdateSlot(const QString &);

signals:
    void pathUpdated(const QString &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FILECHANNELPARAMETERVIEW_H

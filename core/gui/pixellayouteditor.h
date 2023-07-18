#ifndef PHOTON_PIXELLAYOUTEDITOR_H
#define PHOTON_PIXELLAYOUTEDITOR_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT PixelLayoutEditor : public QWidget
{
public:
    PixelLayoutEditor(PixelLayout *, QWidget *parent = nullptr);
    ~PixelLayoutEditor();

    PixelLayout *pixelLayout() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELLAYOUTEDITOR_H

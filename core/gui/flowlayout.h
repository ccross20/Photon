#ifndef PHOTON_FLOWLAYOUT_H
#define PHOTON_FLOWLAYOUT_H

#include <QLayout>
#include <QStyle>
#include "photon-global.h"

namespace photon {

// A layout that places its children left-to-right, wrapping to a new line
// when a row runs out of width - Qt Widgets has no built-in equivalent. Used
// wherever a variable-length row of chips/tags needs to wrap instead of
// overflowing or scrolling horizontally.
class PHOTONCORE_EXPORT FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem*> m_itemList;
    int m_hSpace;
    int m_vSpace;
};

} // namespace photon

#endif // PHOTON_FLOWLAYOUT_H

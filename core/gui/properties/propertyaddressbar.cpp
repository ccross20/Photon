#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "propertyaddressbar.h"

namespace photon {

namespace {

const char *kCrumbStyle =
    "QPushButton { border: none; background: transparent; padding: 0px;"
    " color: #8fc7ff; text-decoration: underline; }"
    "QPushButton:hover { color: #ffffff; }";

} // namespace

PropertyAddressBar::PropertyAddressBar(QWidget *t_parent) : QWidget(t_parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
    rebuild();
}

void PropertyAddressBar::setAddress(const PropertyAddress &t_address)
{
    m_address = t_address;
    rebuild();
}

void PropertyAddressBar::rebuild()
{
    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    if (m_address.isEmpty()) {
        QLabel *empty = new QLabel(QStringLiteral("No selection"));
        empty->setStyleSheet("color: #777777;");
        m_layout->addWidget(empty);
        m_layout->addStretch(1);
        return;
    }

    const QVector<PropertyAddress::Segment> segments = m_address.segments();
    for (int i = 0; i < segments.size(); ++i) {
        const PropertyAddress::Segment &segment = segments.at(i);
        const bool isLast = (i == segments.size() - 1);

        if (isLast) {
            QLabel *current = new QLabel(segment.label);
            current->setStyleSheet("color: #f0f0f0; font-weight: bold;");
            m_layout->addWidget(current);
        } else {
            QPushButton *crumb = new QPushButton(segment.label);
            crumb->setFlat(true);
            crumb->setCursor(Qt::PointingHandCursor);
            crumb->setStyleSheet(kCrumbStyle);
            connect(crumb, &QPushButton::clicked, this, [this, segment, i]() {
                emit segmentActivated(segment, i);
            });
            m_layout->addWidget(crumb);

            QLabel *separator = new QLabel(QStringLiteral(">"));
            separator->setStyleSheet("color: #777777;");
            m_layout->addWidget(separator);
        }
    }

    m_layout->addStretch(1);
}

} // namespace photon

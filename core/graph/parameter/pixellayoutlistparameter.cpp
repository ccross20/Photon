#include <QListWidget>
#include <QLabel>
#include <QCoreApplication>
#include <QJsonArray>
#include "pixellayoutlistparameter.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixellayout.h"

namespace photon {

const QByteArray PixelLayoutListParameter::ParameterId = "pixelLayoutList";

PixelLayoutListParameter::PixelLayoutListParameter() : Parameter() {}

PixelLayoutListParameter::PixelLayoutListParameter(const QByteArray &t_id, const QString &t_name,
                                                   QStringList t_default, int connectionOptions)
    : Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{
}

QWidget *PixelLayoutListParameter::createWidget(keira::NodeEditor *) const
{
    auto *app = qobject_cast<PhotonCore *>(QCoreApplication::instance());
    if (!app || !app->project())
        return new QLabel("No project");

    auto *list = new QListWidget;
    list->setMaximumHeight(120);

    const QStringList selected = value().toStringList();
    for (auto *layout : app->project()->pixelLayouts()->layouts()) {
        const QString id = QString::fromLatin1(layout->uniqueId());
        auto *item = new QListWidgetItem(layout->name().isEmpty() ? QStringLiteral("(unnamed)") : layout->name());
        item->setData(Qt::UserRole, id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(selected.contains(id) ? Qt::Checked : Qt::Unchecked);
        list->addItem(item);
    }

    auto *param = const_cast<PixelLayoutListParameter *>(this);
    QObject::connect(list, &QListWidget::itemChanged, [list, param]() {
        param->setValue(param->updateValue(list));
    });
    return list;
}

QVariant PixelLayoutListParameter::updateValue(QWidget *t_widget) const
{
    QStringList ids;
    if (auto *list = qobject_cast<QListWidget *>(t_widget)) {
        for (int i = 0; i < list->count(); ++i) {
            auto *item = list->item(i);
            if (item->checkState() == Qt::Checked)
                ids << item->data(Qt::UserRole).toString();
        }
    }
    return ids;
}

void PixelLayoutListParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    QStringList ids;
    for (const auto &v : t_json.value("layouts").toArray())
        ids << v.toString();
    setValue(ids);
}

void PixelLayoutListParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    QJsonArray arr;
    for (const auto &id : value().toStringList())
        arr.append(id);
    t_json.insert("layouts", arr);
}

} // namespace photon

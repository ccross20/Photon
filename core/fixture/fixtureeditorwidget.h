#ifndef PHOTON_FIXTUREEDITORWIDGET_H
#define PHOTON_FIXTUREEDITORWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class FixtureEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FixtureEditorWidget(QWidget *parent = nullptr);
    ~FixtureEditorWidget();

    void setFixtures(QVector<Fixture*> fixtures);

signals:

private slots:
    void setName(const QString &name);
    void setDefinition(const QString &path);
    void updateComments();
    void setIdentifier(const QString &);
    void setUniverse(uint universe);
    void setOffset(uchar channel);
    void setMode(int index);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);


private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_FIXTUREEDITORWIDGET_H

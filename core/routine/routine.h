#ifndef PHOTON_ROUTINE_H
#define PHOTON_ROUTINE_H

#include "model/graph.h"
#include "photon-global.h"
#include "project/projectresource.h"
#include "sequence/channel.h"
#include "opengl/openglframebuffer.h"

namespace photon {

class PHOTONCORE_EXPORT Routine : public keira::Graph, public ProjectResource
{
    Q_OBJECT
public:



    explicit Routine(const QString &t_name = QString{}, QObject *parent = nullptr);
    ~Routine();

    void initializeContext(QOpenGLContext *, Canvas *);
    void canvasResized(QOpenGLContext *, Canvas *);

    QString name() const;
    void setName(const QString &);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "routine";}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override;

    void addChannel(const ChannelInfo &info);
    void updateChannel(int index, const ChannelInfo &info);
    void removeChannel(int index);
    int channelCount() const;
    ChannelInfo channelInfoAtIndex(int index);
    const QVector<ChannelInfo> &channelInfo() const;
    void evaluate(keira::EvaluationContext *) const override;


    void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;
    void writeToJson(QJsonObject &) const override;

signals:
    void channelAdded(int index);
    void channelRemoved(int index);
    void channelUpdated(int index);

protected:

    void nodeAdded(keira::Node *) override;
    void nodeRemoved(keira::Node *) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ROUTINE_H

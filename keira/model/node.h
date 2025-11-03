#ifndef KEIRA_NODE_H
#define KEIRA_NODE_H

#include "keira-global.h"

namespace keira {


struct KEIRA_EXPORT NodeInformation
{

    NodeInformation(){}
    NodeInformation(std::function<Node*()> _callback):callback(_callback){}

    QByteArray nodeId;
    QByteArray translateId;
    QByteArray iconId;
    QByteArray nodeCollectionId;
    QString name;
    std::function<Node*()> callback;

    NodeCategoryList categories;
};

struct KEIRA_EXPORT EvaluationContext
{
    virtual ~EvaluationContext(){}
    uint loopCount = 0;
};

class KEIRA_EXPORT Node
{
public:
    explicit Node(const QByteArray &id);
    virtual ~Node();

    virtual void createParameters();
    Graph *graph() const;
    void setName(const QString &);
    QString name() const;
    void setId(const QByteArray &);
    QByteArray id() const;
    QByteArray uniqueId() const;
    void setIsAlwaysDirty(bool);
    bool isAlwaysDirty() const;
    void setWidth(double);
    double width() const;
    void addParameter(Parameter *);
    void removeParameter(Parameter *);
    void updateParameter(Parameter *);
    Parameter *findParameter(const QByteArray &) const;
    const QVector<Parameter*> &parameters() const;

    virtual void setValue(const QByteArray &t_id, const QVariant &t_value);
    const QVariant &value(const QByteArray &t_id) const;

    QPointF position() const;
    void setPosition(const QPointF &);

    virtual void evaluate(EvaluationContext *context) const;
    virtual void buttonClicked(const Parameter *);
    virtual void markDirty();
    void markClean();
    bool isDirty() const;
    bool isLoopable() const;
    virtual uint loopCount() const;
    virtual void startLoop();
    virtual void endLoop();

    virtual void readFromJson(const QJsonObject &, NodeLibrary *library);
    virtual void writeToJson(QJsonObject &) const;

protected:
    virtual void inputParameterConnected(Parameter*);
    virtual void outputParameterConnected(Parameter*);
    virtual void inputParameterDisconnected(Parameter*);
    virtual void outputParameterDisconnected(Parameter*);
    void setIsLoopable(bool);

private:
    friend class GraphSorter;
    friend class Graph;

    class Impl;
    Impl *m_impl;
};

} // namespace keira


#endif // KEIRA_NODE_H

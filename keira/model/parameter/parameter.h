#ifndef KEIRA_PARAMETER_H
#define KEIRA_PARAMETER_H
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT Parameter
{
public:
    enum LayoutOptions
    {
        LayoutNone,
        LayoutNoLabel = 0x1
    };


    Parameter();
    Parameter(const QByteArray &t_typeId, const QByteArray &t_id, const QString &t_name, const QVariant &t_default, int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    virtual ~Parameter();

    QByteArray id() const;
    virtual QByteArray typeId() const;
    virtual bool isGeneric() const;
    virtual bool acceptsConnectionFrom(const Parameter *source) const;
    QString name() const;
    QString description() const;
    bool hasInput() const;
    bool hasOutput() const;
    bool allowMultipleInput() const;
    bool allowMultipleOutput() const;
    bool allowInput() const;
    bool allowOutput() const;
    bool isReadOnly() const;
    void setConnectionOptions(int);

    bool testValueForChange() const;
    void setTestValueForChange(bool);

    int layoutOptions() const;
    void setLayoutOptions(int);
    void setName(const QString &);
    void setId(const QByteArray &);

    Node *node() const;
    bool canConvert(const QByteArray &) const;
    QVariant convertTo(const QByteArray &) const;

    const QVector<Parameter*> &outputParameters() const;
    const QVector<Parameter*> &inputParameters() const;
    Parameter *inputParameter() const;

    const QVariant &value() const;
    virtual void setValue(const QVariant &);

    int rowHeight() const;
    bool isDirty() const;

    Parameter *clone(NodeLibrary *library) const;
    virtual QWidget *createWidget(NodeEditor *) const;
    virtual void updateWidget(QWidget *) const;
    virtual QVariant updateValue(QWidget *) const;

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

protected:
    virtual void validate(const QVariant &);
    void markDirty();

private:
    friend class Node;
    friend class Graph;

    class Impl;
    Impl *m_impl;
};

} // namespace keira


Q_DECLARE_METATYPE(keira::Parameter);

#endif // KEIRA_PARAMETER_H

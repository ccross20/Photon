#ifndef PARAMETER_P_H
#define PARAMETER_P_H

#include "parameter.h"

namespace keira
{
    class Parameter::Impl
    {
    public:
        Impl(Parameter*);
        void connectOutput(Parameter *);
        void connectInput(Parameter *);
        void disconnectOutput(Parameter *);
        void disconnectInput(Parameter *);
        int connectionOptions = 0;
        QByteArray id;
        QByteArray typeId;
        QString name;
        QString description;
        Node *node = nullptr;
        QVector<Parameter*> inputParams;
        QVector<Parameter*> outputParams;
        QVariant value;
        QVariant defaultValue;
        int layoutOptions = LayoutNone;
        bool isDirty = true;
        Parameter *facade = nullptr;

    };
}

#endif // PARAMETER_P_H

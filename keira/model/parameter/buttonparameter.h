#ifndef KEIRA_BUTTONPARAMETER_H
#define KEIRA_BUTTONPARAMETER_H
#include "parameter.h"


namespace keira {

class KEIRA_EXPORT ButtonParameter : public Parameter
{
public:
    const static QByteArray ParameterId;

    ButtonParameter();
    ButtonParameter(const QByteArray &t_id, const QString &t_name);


    QWidget *createWidget(NodeItem *) const override;
};

} // namespace photon

#endif // KEIRA_BUTTONPARAMETER_H

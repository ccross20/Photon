#ifndef KEIRA_EVALUATIONTEST_H
#define KEIRA_EVALUATIONTEST_H

#include <QObject>

namespace keira {

class EvaluationTest : public QObject
{
    Q_OBJECT
public:
    explicit EvaluationTest(QObject *parent = nullptr);

private slots:
    void simpleEval() const;

signals:


};

} // namespace keira

#endif // KEIRA_EVALUATIONTEST_H

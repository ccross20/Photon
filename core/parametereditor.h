#ifndef PARAMETEREDITOR_H
#define PARAMETEREDITOR_H

#include <QWidget>

class ParameterEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ParameterEditor(QWidget *parent = nullptr);

signals:
    void beamChanged(int);
    void segmentChanged(double);
    void radiusChanged(double);
    void offsetChanged(double);
    void lengthChanged(double);
    void angleChanged(double);

};

#endif // PARAMETEREDITOR_H

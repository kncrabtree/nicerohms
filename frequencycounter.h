#ifndef FREQUENCYCOUNTER_H
#define FREQUENCYCOUNTER_H

#include "hardwareobject.h"
#include <QTimer>

class FrequencyCounter : public HardwareObject
{
    Q_OBJECT
public:
    FrequencyCounter(QObject *partent = nullptr);
    virtual ~FrequencyCounter();

signals:
    void freqUpdate(double);

public slots:
    void readTimerInterval();
    virtual double read() = 0;

protected:
    double d_currentFreq;

    QTimer *p_timer;
    bool d_scanActive;

public slots:
    virtual void beginAcquisition();
    virtual void endAcquisition();




};

#include "fc8150.h"
class FC8150;
typedef FC8150 CounterHardware;

#endif // FREQUENCYCOUNTER_H

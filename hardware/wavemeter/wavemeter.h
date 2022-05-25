#ifndef WAVEMETER_H
#define WAVEMETER_H

#include <hardware/core/hardwareobject.h>

#include <QTimer>

//all frequencies should be in Hz!

class Wavemeter : public HardwareObject
{
	Q_OBJECT
public:
	Wavemeter(QObject *parent = nullptr);
    virtual ~Wavemeter();

signals:
    void freqUpdate(double);

public slots:
	void readTimerInterval();
    virtual double read() =0;

protected:
    double d_currentFreq;

	QTimer *p_timer;
	bool d_scanActive;

	// HardwareObject interface
public slots:
	virtual void beginAcquisition();
	virtual void endAcquisition();
};

#if NICEROHMS_WAVEMETER == 1
#include "bristol621.h"
class Bristol621;
typedef Bristol621 WavemeterHardware;
#else
#include "virtualwavemeter.h"
class VirtualWavemeter;
typedef VirtualWavemeter WavemeterHardware;
#endif

#endif // WAVEMETER_H

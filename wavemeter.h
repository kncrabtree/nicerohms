#ifndef WAVEMETER_H
#define WAVEMETER_H

#include "hardwareobject.h"

#include <QTimer>

//all frequencies should be in Hz!

class Wavemeter : public HardwareObject
{
	Q_OBJECT
public:
	Wavemeter(QObject *parent = nullptr);
	~Wavemeter();

    enum WavemeterState {
        Unknown,
        Pump,
        Signal
    };

signals:
	void signalUpdate(double);
	void pumpUpdate(double);
	void switchRequest();

public slots:
	void readTimerInterval();
    Wavemeter::WavemeterState getState() const;
    virtual double read() =0;
    virtual void flipComplete();

protected:
	double d_signalFreq, d_pumpFreq;
	WavemeterState d_currentState;

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

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

signals:
	void signalUpdate(double);
	void pumpUpdate(double);
	void switchRequest();

public slots:
	void readSignal();
	void readPump();
	void switchComplete();
	void readTimerInterval();

protected:
	enum WavemeterState {
		Unknown,
		Pump,
		Signal
	};

	double d_signalFreq, d_pumpFreq;
	WavemeterState d_currentState;


	virtual double read() =0;
	QTimer *p_timer;
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

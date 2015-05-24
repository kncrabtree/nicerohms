#ifndef IOBOARD_H
#define IOBOARD_H

#include "hardwareobject.h"

class IOBoard : public HardwareObject
{
	Q_OBJECT
public:
	IOBoard(QObject *parent = nullptr);
	~IOBoard();

signals:
	void mirrorFlipped();
	void relockComplete(bool success);
	void lockState(bool);

public slots:
	virtual void flipWavemeterMirror() =0;
	virtual bool readCavityLocked() =0;
	void relock();
	void readRelockSettings();

protected:
	double d_lastCavityVoltage, d_relockStep;

	virtual void setCavityPZTVoltage(double v) =0;
	virtual void setCavityLockOverride(bool unlock) =0;

};

#if NICEROHMS_IOBOARD == 1
#include "ioboardu6.h"
class IOBoardU6;
typedef IOBoardU6 IOBoardHardware;

#define NICEROHMS_IOB_WM 0
#define NICEROHMS_IOB_LOCK 1
#define NICEROHMS_IOB_OVERRIDE 2
#define NICEROHMS_IOB_CV 0
#else
#include "virtualioboard.h"
class VirtualIOBoard;
typedef VirtualIOBoard IOBoardHardware;

#define NICEROHMS_IOB_WM 0
#define NICEROHMS_IOB_LOCK 1
#define NICEROHMS_IOB_OVERRIDE 2
#define NICEROHMS_IOB_CV 0
#endif

#endif // IOBOARD_H

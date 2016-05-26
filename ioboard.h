#ifndef IOBOARD_H
#define IOBOARD_H

#include "hardwareobject.h"

#include <QTimer>

class IOBoard : public HardwareObject
{
	Q_OBJECT
public:
	IOBoard(QObject *parent = nullptr);
    virtual ~IOBoard();

signals:
	void relockComplete(bool success);
	void lockState(bool);

public slots:
    virtual bool readCavityLocked() =0;
    virtual void relock() = 0;
    virtual void relockPump() = 0;

	void readIOSettings();

protected:
	double d_lastCavityVoltage, d_relockStep;
	int d_readLockInterval;
	QTimer *p_lockReadTimer;

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

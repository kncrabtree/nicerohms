#ifndef LASER_H
#define LASER_H

#include "hardwareobject.h"

class QTimer;

class Laser : public HardwareObject
{
	Q_OBJECT
public:
	Laser(QObject *parent = nullptr);
	~Laser();

signals:
	void laserPosChanged(double);
	void slewStarting();
	void slewComplete();

public slots:
	void readSlewParameters();
	void slewToPosition(double target);
	void nextSlewPoint();
	virtual double readPosition() =0;

protected:
	bool d_slewing;
	double d_currentPos, d_slewStep, d_slewTarget;
	int d_slewInterval;
	virtual double setPosition(double target) =0;

	QTimer *p_slewTimer;

};

#if NICEROHMS_LASER == 1
#include "lasermtd694b.h"
class LaserMDT694B;
typedef LaserMDT694B LaserHardware;
#else
#include "virtuallaser.h"
class VirtualLaser;
typedef VirtualLaser LaserHardware;
#endif


#endif // LASER_H

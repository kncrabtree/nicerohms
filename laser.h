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
	virtual double readPosition() =0;

protected:
	bool d_slewing;
	double d_currentPos, d_slewStep;
	int d_slewInterval;
	virtual double setPosition(double target) =0;

	QTimer *p_slewTimer;

};

#if NICEROHMS_LASER == 1
#include "mtd694b.h"
class MDT694B;
typedef MDT694B LaserHardware;
#else
#include "virtuallaser.h"
class VirtualLaser;
typedef VirtualLaser LaserHardware;
#endif


#endif // LASER_H

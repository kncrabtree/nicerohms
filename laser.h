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

private:
	QTimer *p_slewTimer;
};

#endif // LASER_H

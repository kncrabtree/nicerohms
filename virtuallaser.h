#ifndef VIRTUALLASER_H
#define VIRTUALLASER_H

#include "laser.h"

class VirtualLaser : public Laser
{
	Q_OBJECT
public:
	VirtualLaser(QObject *parent = nullptr);
	~VirtualLaser();

	// HardwareObject interface
public slots:
	bool testConnection();
	void initialize();
	Scan prepareForScan(Scan scan);
	void beginAcquisition();
	void endAcquisition();
	void readPointData();

	// Laser interface
public slots:
	double readPosition();
	double estimateFrequency();

protected:
	double setPosition(double target);
};

#endif // VIRTUALLASER_H

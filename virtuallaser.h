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
    bool testConnection() override;
    void initialize() override;
    Scan prepareForScan(Scan scan) override;
    void beginAcquisition() override;
    void endAcquisition() override;
    void readPointData() override;

	// Laser interface
public slots:
    double readPosition() override;
    double estimateFrequency() override;

protected:
    double setPosition(double target) override;

    // Laser interface
protected:
    void calibrate() override;
};

#endif // VIRTUALLASER_H

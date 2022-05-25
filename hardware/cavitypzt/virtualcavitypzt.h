#ifndef VIRTUALCAVITYPZT_H
#define VIRTUALCAVITYPZT_H

#include "cavitypztdriver.h"

class VirtualCavityPZT : public CavityPZTDriver
{
	Q_OBJECT
public:
	VirtualCavityPZT(QObject *parent = nullptr);
	~VirtualCavityPZT();

	// HardwareObject interface
public slots:
	bool testConnection();
	void initialize();
	Scan prepareForScan(Scan scan);
	void beginAcquisition();
	void endAcquisition();
	void readPointData();

	// CavityPZTDriver interface
public slots:
	double readVoltage();
};

#endif // VIRTUALCAVITYPZT_H

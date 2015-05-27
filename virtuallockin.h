#ifndef VIRTUALLOCKIN_H
#define VIRTUALLOCKIN_H

#include "lockin.h"

class VirtualLockIn : public LockIn
{
	Q_OBJECT
public:
	VirtualLockIn(int num, QObject *parent = nullptr);
	~VirtualLockIn();

	// HardwareObject interface
public slots:
	bool testConnection();
	void initialize();
	Scan prepareForScan(Scan scan);
	void beginAcquisition();
	void endAcquisition();
	void readPointData();
};

#endif // VIRTUALLOCKIN_H

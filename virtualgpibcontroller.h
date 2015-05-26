#ifndef VIRTUALGPIBCONTROLLER_H
#define VIRTUALGPIBCONTROLLER_H

#include "gpibcontroller.h"

class VirtualGpibController : public GpibController
{
	Q_OBJECT
public:
	VirtualGpibController(QObject *parent = 0);
	~VirtualGpibController();

	// HardwareObject interface
public slots:
	bool testConnection();
	void initialize();
	Scan prepareForScan(Scan scan);
	void beginAcquisition();
	void endAcquisition();
	void readPointData();

	// GpibController interface
protected:
	void readAddress();
	void setAddress(int a);
};

#endif // VIRTUALGPIBCONTROLLER_H

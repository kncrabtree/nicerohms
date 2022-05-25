#ifndef CAVITYPZTDRIVER_H
#define CAVITYPZTDRIVER_H

#include <hardware/core/hardwareobject.h>

class CavityPZTDriver : public HardwareObject
{
	Q_OBJECT
public:
	CavityPZTDriver(QObject *parent = nullptr);
    virtual ~CavityPZTDriver();

signals:
	void cavityPZTUpdate(double);

public slots:
	virtual double readVoltage() =0;
};

#if NICEROHMS_CAVITYPZT == 1
#include "cavitymdt694b.h"
class CavityMDT694B;
typedef CavityMDT694B CavityPZTHardware;
#else
#include "virtualcavitypzt.h"
class VirtualCavityPZT;
typedef VirtualCavityPZT CavityPZTHardware;
#endif

#endif // CAVITYPZTDRIVER_H

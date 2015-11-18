#ifndef GPIBCONTROLLER_H
#define GPIBCONTROLLER_H

#include "hardwareobject.h"


class GpibController : public HardwareObject
{
	Q_OBJECT
public:
	GpibController(QObject *parent = nullptr);
    virtual ~GpibController();

	bool writeCmd(int address, QString cmd);
	QByteArray queryCmd(int address, QString cmd);

protected:
    virtual void readAddress() =0;
	virtual void setAddress(int a) =0;

	int d_currentAddress;
};

#if NICEROHMS_GPIBCONTROLLER == 1
#include "prologixusb.h"
class PrologixUSB;
typedef PrologixUSB GpibControllerHardware;
#else
#include "virtualgpibcontroller.h"
class VirtualGpibController;
typedef VirtualGpibController GpibControllerHardware;
#endif

#endif // GPIBCONTROLLER_H

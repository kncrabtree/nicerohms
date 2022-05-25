#ifndef LOCKIN_H
#define LOCKIN_H

#include <hardware/core/hardwareobject.h>

class LockIn : public HardwareObject
{
	Q_OBJECT
public:
	LockIn(int num, QObject *parent = nullptr);
    virtual ~LockIn();

protected:
	const int d_id;
};

#ifdef NICEROHMS_VIRTUALLOCKIN
#ifndef VIRTUALLOCKIN_H
#include "virtuallockin.h"
#endif
class VirtualLockIn;
#endif

#ifdef NICEROHMS_LOCKIN_0
using LockIn1Hardware = NICEROHMS_LOCKIN_0;
#endif

#ifdef NICEROHMS_LOCKIN_1
using LockIn2Hardware = NICEROHMS_LOCKIN_1;
#endif


#endif // LOCKIN_H

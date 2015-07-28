#ifndef LOCKIN_H
#define LOCKIN_H

#include "hardwareobject.h"

class LockIn : public HardwareObject
{
	Q_OBJECT
public:
	LockIn(int num, QObject *parent = nullptr);
    virtual ~LockIn();

protected:
	const int d_id;
};

#if NICEROHMS_LOCKIN1 == 1
#include "sr530.h"
class SR530;
typedef SR530 LockIn1Hardware;
#else
#include "virtuallockin.h"
class VirtualLockIn;
typedef VirtualLockIn LockIn1Hardware;
#endif

#if NICEROHMS_LOCKIN2 == 1
#include "sr530.h"
class SR530;
typedef SR530 LockIn2Hardware;
#else
#include "virtuallockin.h"
class VirtualLockIn;
typedef VirtualLockIn LockIn2Hardware;
#endif


#endif // LOCKIN_H

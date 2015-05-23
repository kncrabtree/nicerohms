#include "lockin.h"

LockIn::LockIn(int num, QObject *parent) :
	HardwareObject(parent), d_id(num)
{
	d_key = QString("lockin%1").arg(d_id);
}

LockIn::~LockIn()
{

}


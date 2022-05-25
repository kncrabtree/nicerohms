#include <hardware/core/hardwareobject.h>

HardwareObject::HardwareObject(QObject *parent) :
    QObject(parent), d_isCritical(true), d_isActive(false)
{
}

HardwareObject::~HardwareObject()
{

}


void HardwareObject::sleep(bool b)
{
	if(b)
		emit logMessage(name().append(QString(" is asleep.")));
	else
		emit logMessage(name().append(QString(" is awake.")));
}

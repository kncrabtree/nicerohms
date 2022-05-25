#include "cavitypztdriver.h"

CavityPZTDriver::CavityPZTDriver(QObject *parent) :
	HardwareObject(parent)
{
	d_key = QString("cavityPZT");
	d_isCritical = false;
}

CavityPZTDriver::~CavityPZTDriver()
{

}


#include "cavitypztdriver.h"

CavityPZTDriver::CavityPZTDriver(QObject *parent) :
	HardwareObject(parent)
{
	d_key = QString("cavityPZT");
}

CavityPZTDriver::~CavityPZTDriver()
{

}


#include "gpibcontroller.h"
#include <QDebug>

GpibController::GpibController(QObject *parent) :
	HardwareObject(parent)
{
	d_key = QString("gpibController");
}

GpibController::~GpibController()
{

}

bool GpibController::writeCmd(int address, QString cmd)
{
	if(address != d_currentAddress)
		setAddress(address);
    qDebug() << d_currentAddress;
	return p_comm->writeCmd(cmd);
}

QByteArray GpibController::queryCmd(int address, QString cmd)
{
	if(address != d_currentAddress)
		setAddress(address);

	return p_comm->queryCmd(cmd);
}


#include "virtuallockin.h"

#include "virtualinstrument.h"

VirtualLockIn::VirtualLockIn(int num, QObject *parent) :
	LockIn(num,parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual LockIn %1").arg(d_id);
    d_isCritical = false;

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualLockIn::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualLockIn::~VirtualLockIn()
{

}



bool VirtualLockIn::testConnection()
{
	emit connected();
	return true;
}

void VirtualLockIn::initialize()
{
	testConnection();
}

Scan VirtualLockIn::prepareForScan(Scan scan)
{
	if(d_isActive)
		scan.addNumDataPoints(2);

	return scan;

}

void VirtualLockIn::beginAcquisition()
{
}

void VirtualLockIn::endAcquisition()
{
}

void VirtualLockIn::readPointData()
{
	if(d_isActive)
	{
		double x = (static_cast<double>(qrand() % 2000)-1000.0)/100.0;
		double y = (static_cast<double>(qrand() % 2000)-1000.0)/100.0;

		QList<QPair<QString,QVariant>> out;
		out.append(qMakePair(QString("%1X").arg(d_key),QVariant::fromValue(x)));
		out.append(qMakePair(QString("%1Y").arg(d_key),QVariant::fromValue(y)));

		emit pointDataRead(out);
	}
}

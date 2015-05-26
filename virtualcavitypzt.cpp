#include "virtualcavitypzt.h"

#include "virtualinstrument.h"

VirtualCavityPZT::VirtualCavityPZT(QObject *parent) :
	CavityPZTDriver(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual Cavity PZT Driver");

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualCavityPZT::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualCavityPZT::~VirtualCavityPZT()
{

}



bool VirtualCavityPZT::testConnection()
{
	readVoltage();

	emit connected();
	return true;
}

void VirtualCavityPZT::initialize()
{
	testConnection();
}

Scan VirtualCavityPZT::prepareForScan(Scan scan)
{
	scan.addNumDataPoints(1);
	return scan;
}

void VirtualCavityPZT::beginAcquisition()
{
}

void VirtualCavityPZT::endAcquisition()
{
}

void VirtualCavityPZT::readPointData()
{
	QList<QPair<QString,QVariant>> out;
	out.append(qMakePair(QString("CavityPZT"),readVoltage()));
	emit pointDataRead(out);
}

double VirtualCavityPZT::readVoltage()
{
	emit cavityPZTUpdate(50.0);
	return 50.0;
}

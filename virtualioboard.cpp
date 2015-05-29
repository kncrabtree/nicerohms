#include "virtualioboard.h"

#include "virtualinstrument.h"

VirtualIOBoard::VirtualIOBoard(QObject *parent) :
	IOBoard(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual IO Board");

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualIOBoard::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualIOBoard::~VirtualIOBoard()
{

}



bool VirtualIOBoard::testConnection()
{
	p_lockReadTimer->stop();
	readIOSettings();
	readCavityLocked();

	p_lockReadTimer->start();
	emit connected();
	return true;
}

void VirtualIOBoard::initialize()
{
	testConnection();
}

Scan VirtualIOBoard::prepareForScan(Scan scan)
{
	readIOSettings();

	return scan;
}

void VirtualIOBoard::beginAcquisition()
{
}

void VirtualIOBoard::endAcquisition()
{
}

void VirtualIOBoard::readPointData()
{
	//do something here
}

bool VirtualIOBoard::readCavityLocked()
{
	bool on = qrand() % 1000;
	emit lockState(on);
	return on;
}

void VirtualIOBoard::setCavityPZTVoltage(double v)
{
	d_lastCavityVoltage = v;
}

void VirtualIOBoard::setCavityLockOverride(bool unlock)
{
	Q_UNUSED(unlock)
}

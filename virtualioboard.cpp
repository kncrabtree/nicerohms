#include "virtualioboard.h"

#include "virtualinstrument.h"

VirtualIOBoard::VirtualIOBoard(QObject *parent) :
	IOBoard(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual IO Board");

	d_comm = new VirtualInstrument(d_key,this);
	connect(d_comm,&CommunicationProtocol::logMessage,this,&VirtualIOBoard::logMessage);
	connect(d_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualIOBoard::~VirtualIOBoard()
{

}



bool VirtualIOBoard::testConnection()
{
	readRelockSettings();

	emit connected();
	return true;
}

void VirtualIOBoard::initialize()
{
	testConnection();
}

Scan VirtualIOBoard::prepareForScan(Scan scan)
{
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

void VirtualIOBoard::flipWavemeterMirror()
{
	emit mirrorFlipped();
}

bool VirtualIOBoard::readCavityLocked()
{
	emit lockState(true);
	return true;
}

void VirtualIOBoard::setCavityPZTVoltage(double v)
{
	d_lastCavityVoltage = v;
}

void VirtualIOBoard::setCavityLockOverride(bool unlock)
{
	Q_UNUSED(unlock)
}

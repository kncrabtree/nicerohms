#include "virtuallaser.h"

#include "virtualinstrument.h"

VirtualLaser::VirtualLaser(QObject *parent) :
	Laser(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual Laser");

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualLaser::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualLaser::~VirtualLaser()
{

}

bool VirtualLaser::testConnection()
{
	readPosition();
	readSlewParameters();
	emit connected();
	return true;
}

void VirtualLaser::initialize()
{
	testConnection();
}

Scan VirtualLaser::prepareForScan(Scan scan)
{
	readSlewParameters();
	if(d_isActive)
		scan.addNumDataPoints(1);

	return scan;
}

void VirtualLaser::beginAcquisition()
{

}

void VirtualLaser::endAcquisition()
{

}

void VirtualLaser::readPointData()
{
	if(d_isActive)
	{
		QList<QPair<QString,QVariant>> out;
		out.append(qMakePair(QString("laser"),readPosition()));
		emit pointDataReadNoPlot(out);
	}
}

double VirtualLaser::readPosition()
{
	emit laserPosChanged(d_currentPos);
	return d_currentPos;
}

double VirtualLaser::setPosition(double target)
{

	d_currentPos = target;
	return readPosition();
}


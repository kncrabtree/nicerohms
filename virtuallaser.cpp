#include "virtuallaser.h"

#include "virtualinstrument.h"

VirtualLaser::VirtualLaser(QObject *parent) :
	Laser(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual Laser");
    d_isCritical = true;

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualLaser::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    s.setValue(QString("%1/%2/controlStep").arg(d_key).arg(d_subKey),1.0);
    s.setValue(QString("%1/%2/decimals").arg(d_key).arg(d_subKey),3);
    s.setValue(QString("%1/%2/units").arg(d_key).arg(d_subKey),QString("V"));
    s.setValue(QString("%1/%2/minPos").arg(d_key).arg(d_subKey),0.0);
    s.setValue(QString("%1/%2/maxPos").arg(d_key).arg(d_subKey),150.0);
    s.sync();
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

double VirtualLaser::estimateFrequency()
{
    //normally would want to read position here
	return 29.9792458/0.0001064*1e9 + d_currentPos/150.0*1e11;
}

double VirtualLaser::setPosition(double target)
{

	d_currentPos = target;
	return readPosition();
}


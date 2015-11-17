#include "ioboard.h"

IOBoard::IOBoard(QObject *parent) :
	HardwareObject(parent), d_lastCavityVoltage(0.0), d_readLockInterval(5)
{
	d_key = QString("ioboard");

	p_lockReadTimer = new QTimer(this);
	connect(p_lockReadTimer,&QTimer::timeout,this,&IOBoard::readCavityLocked);
	connect(this,&IOBoard::hardwareFailure,p_lockReadTimer,&QTimer::stop);
}

IOBoard::~IOBoard()
{

}
/*
void IOBoard::relock()
{
    emit relockComplete(true);
}
*/
void IOBoard::readIOSettings()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	d_relockStep = s.value(QString("%1/%2/relockStep").arg(d_key).arg(d_subKey),0.1).toDouble();
    d_readLockInterval = s.value(QString("%1/%2/lockReadInterval"),5).toInt();
}


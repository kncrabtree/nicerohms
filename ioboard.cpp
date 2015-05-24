#include "ioboard.h"

IOBoard::IOBoard(QObject *parent) :
	HardwareObject(parent), d_lastCavityVoltage(0.0)
{
}

IOBoard::~IOBoard()
{

}

void IOBoard::relock()
{
	emit relockComplete(true);
}

void IOBoard::readRelockSettings()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	d_relockStep = s.value(QString("%1/%2/relockStep").arg(d_key).arg(d_subKey),0.1).toDouble();
}


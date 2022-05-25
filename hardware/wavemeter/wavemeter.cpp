#include "wavemeter.h"

Wavemeter::Wavemeter(QObject *parent) :
	HardwareObject(parent), d_scanActive(false)
{
	d_key = QString("wavemeter");
	d_isCritical = false;

	p_timer = new QTimer(this);
	connect(p_timer,&QTimer::timeout,this,&Wavemeter::read);
	connect(this,&HardwareObject::hardwareFailure,p_timer,&QTimer::stop);
}

Wavemeter::~Wavemeter()
{

}

void Wavemeter::readTimerInterval()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	int timerInterval = s.value(QString("%1/%2/readInterval").arg(d_key).arg(d_subKey),200).toInt();
    p_timer->setInterval(timerInterval);
}


void Wavemeter::beginAcquisition()
{
	p_timer->stop();
	d_scanActive = true;
}

void Wavemeter::endAcquisition()
{
	p_timer->start();
	d_scanActive = false;
}

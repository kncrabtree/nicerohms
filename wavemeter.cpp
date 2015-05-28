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

Wavemeter::WavemeterState Wavemeter::getState() const
{
    return d_currentState;
}

void Wavemeter::flipComplete()
{
    //generally, this should do nothing!
    //it's only here so that the virtual implementation works properly
    //however, it's here and can be reimplemented in a derived class if needed
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

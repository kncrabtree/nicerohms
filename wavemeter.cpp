#include "wavemeter.h"

Wavemeter::Wavemeter(QObject *parent) :
	HardwareObject(parent), d_scanActive(false)
{
	d_key = QString("wavemeter");

	p_timer = new QTimer(this);
	connect(p_timer,&QTimer::timeout,this,&Wavemeter::read);
}

Wavemeter::~Wavemeter()
{

}

void Wavemeter::readSignal()
{
	switch (d_currentState) {
	case Signal:
		read();
		break;
	case Pump:
		if(!d_scanActive)
			p_timer->stop();
		emit switchRequest();
		break;
	case Unknown:
	default:
		if(!d_scanActive)
			p_timer->stop();
		read();
		if(d_currentState == Pump)
			emit switchRequest();
		else if(!d_scanActive)
			p_timer->start();
		break;
	}
}

void Wavemeter::readPump()
{
	switch (d_currentState) {
	case Pump:
		read();
		break;
	case Signal:
		if(!d_scanActive)
			p_timer->stop();
		emit switchRequest();
		break;
	case Unknown:
	default:
		if(!d_scanActive)
			p_timer->stop();
		read();
		if(d_currentState == Signal)
			emit switchRequest();
		else if(!d_scanActive)
			p_timer->start();
		break;
	}
}

void Wavemeter::switchComplete()
{
	if(d_currentState == Pump)
		d_currentState = Signal;
	else if(d_currentState == Signal)
		d_currentState = Pump;

	//if the state is unknown, it will be determined in the read call if possible
	read();

	if(!d_scanActive)
		p_timer->start();
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

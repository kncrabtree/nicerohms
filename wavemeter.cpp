#include "wavemeter.h"

Wavemeter::Wavemeter(QObject *parent) :
	HardwareObject(parent)
{
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
		emit switchRequest();
		break;
	case Unknown:
	default:
		read();
		if(d_currentState == Pump)
			emit switchRequest();
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
		emit switchRequest();
		break;
	case Unknown:
	default:
		read();
		if(d_currentState == Signal)
			emit switchRequest();
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
}

void Wavemeter::readTimerInterval()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	int timerInterval = s.value(QString("%1/%2/readInterval").arg(d_key).arg(d_subKey),200).toInt();
	p_timer->setInterval(timerInterval);
}


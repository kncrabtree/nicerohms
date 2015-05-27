#include "virtualwavemeter.h"

#include "virtualinstrument.h"

VirtualWavemeter::VirtualWavemeter(QObject *parent) :
	Wavemeter(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual Wavemeter");

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualWavemeter::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualWavemeter::~VirtualWavemeter()
{

}



bool VirtualWavemeter::testConnection()
{
	p_timer->stop();

	d_currentState = Pump;
	d_pumpFreq = 2.8175983e14;
	d_signalFreq = 2.0e14;

	readPump();
	readSignal();
	readTimerInterval();

	p_timer->start();
	emit connected();
	return true;

}

void VirtualWavemeter::initialize()
{
	testConnection();
}

Scan VirtualWavemeter::prepareForScan(Scan scan)
{
	if(d_isActive)
		scan.addNumDataPoints(1);
	return scan;
}

void VirtualWavemeter::readPointData()
{
	if(d_isActive)
	{
		readPump();
		QList<QPair<QString,QVariant>> out;
		out.append(qMakePair(QString("wavemeter"),QVariant::fromValue(d_pumpFreq/29979245800)));
		emit pointDataRead(out);
	}
}

double VirtualWavemeter::read()
{
	//this would be where we actually read the wavemeter and determine whether the result is signal or pump
	//for the virtual case, we'll just do nothing

	if(d_currentState == Unknown)
		d_currentState = Pump;

	if(d_currentState == Signal)
	{
		emit signalUpdate(d_signalFreq);
		return d_signalFreq;
	}
	else
	{
		emit pumpUpdate(d_pumpFreq);
		return d_pumpFreq;
	}

}

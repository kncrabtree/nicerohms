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
    //my suggestion is that if the wavemeter doesn't give a valid reading, just set the state to "unknown," don't emit a signal, and return -1.0

	if(d_currentState == Unknown)
		d_currentState = Pump;

	if(d_currentState == Signal)
	{
        d_signalFreq = 2.0e14 + static_cast<double>((qrand()%10000)-5000.0)*1e6;
		emit signalUpdate(d_signalFreq);
		return d_signalFreq;
	}
	else
	{
        d_pumpFreq = 2.8175983e14 + static_cast<double>((qrand()%10000)-5000.0)*1e6;
		emit pumpUpdate(d_pumpFreq);
		return d_pumpFreq;
	}

}

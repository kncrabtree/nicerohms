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


    d_currentFreq = 8.1723094e13;

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
		QList<QPair<QString,QVariant>> out;
        out.append(qMakePair(QString("wavemeter"),read()));
		emit pointDataRead(out);
	}
}

double VirtualWavemeter::read()
{
    d_currentFreq = 8.1723094e13 + static_cast<double>((qrand()%10000)-5000.0)*1e4;
    emit freqUpdate(d_currentFreq);
    return d_currentFreq;
}

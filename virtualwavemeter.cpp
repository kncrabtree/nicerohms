#include "virtualwavemeter.h"

#include "virtualinstrument.h"
#include <QRandomGenerator>

VirtualWavemeter::VirtualWavemeter(QObject *parent) :
	Wavemeter(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual Wavemeter");
    d_isCritical = false;

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
    auto r = QRandomGenerator::global();
    d_currentFreq = 8.1723094e13 + r->generateDouble()*2e8-1e8;
    emit freqUpdate(d_currentFreq);
    return d_currentFreq;
}

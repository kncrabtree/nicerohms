#include "virtualfreqcomb.h"

#include "virtualinstrument.h"

VirtualFreqComb::VirtualFreqComb(QObject *parent) :
	FrequencyComb(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual Frequency Comb");

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualFreqComb::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualFreqComb::~VirtualFreqComb()
{

}



bool VirtualFreqComb::testConnection()
{
	setRepRate(1e8);
	readComb();

	emit connected();
	return true;
}

void VirtualFreqComb::initialize()
{
	testConnection();
}

Scan VirtualFreqComb::prepareForScan(Scan scan)
{
	if(d_isActive)
	{
		scan.addNumDataPoints(8);
	}

	return scan;
}

void VirtualFreqComb::beginAcquisition()
{
}

void VirtualFreqComb::endAcquisition()
{
}

void VirtualFreqComb::readPointData()
{
	if(d_isActive)
	{
		FreqCombData d = readComb();
		QList<QPair<QString,QVariant>> plotData;
		QList<QPair<QString,QVariant>> noPlotData;

		plotData.append(qMakePair(QString("reprate"),d.repRate()));
		plotData.append(qMakePair(QString("idler"),d.calculatedIdlerFreq()));

		noPlotData.append(qMakePair(QString("offsetBeat"),d.offsetBeat()));
		noPlotData.append(qMakePair(QString("pumpBeat"),d.pumpBeat()));
		noPlotData.append(qMakePair(QString("signalBeat"),d.signalBeat()));
		noPlotData.append(qMakePair(QString("deltaN"),d.deltaN()));
		noPlotData.append(qMakePair(QString("replockv"),d.repRateLockVoltage()));
		noPlotData.append(qMakePair(QString("replocked"),d.repRateLocked()));
	}

}

FreqCombData VirtualFreqComb::readComb()
{
	FreqCombData out;
	//normally, we'd make the network request, and all these would be set by calling out.parseXml()
	out.setRepRate((d_currentDDSFreq + 980e6)/10.0);
	out.setOffsetBeat(30e6 + static_cast<double>((qrand() % 20000) - 10000)/1e6);
	out.setPumpBeat(30e6 + static_cast<double>((qrand() % 20000) - 10000)/1e6);
	out.setSignalBeat(30e6 + static_cast<double>((qrand() % 20000) - 10000)/1e6);
	out.setRepRateLockVoltage(3.0);
	out.setRepRateLocked(true);

	out.setBeatSigns(d_pumpBeatPositive,d_signalBeatPositive);
	if(d_overrideNext)
		out.setDeltaN(d_nextDeltaN,d_currentAOMFreq);
	else
		out.setDeltaN(d_currentIdlerFreq,d_currentAOMFreq);

	d_lastMeasurement = out;
	d_currentDDSFreq = out.repRate()*10.0 - 980e6;

	emit combUpdate(out);
	return out;
}

bool VirtualFreqComb::setRepRate(double target)
{
	//normally, would check to make sure DDS frequency doesn't change too much
	//units are Hz!
	d_currentDDSFreq = (target*10.0)-(980e6);
	emit repRateUpdate(target);
	return true;
}

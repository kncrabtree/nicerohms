#include "virtualaomsynthesizer.h"

#include "gpibinstrument.h"

VirtualAomSynthesizer::VirtualAomSynthesizer(GpibController *c, QObject *parent) :
	AomSynthesizer(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual AOM Synthesizer");

	p_comm = new GpibInstrument(d_key,d_subKey,c,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualAomSynthesizer::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

	d_frequency = 185e6;
}

VirtualAomSynthesizer::~VirtualAomSynthesizer()
{

}



bool VirtualAomSynthesizer::testConnection()
{
	bool success = p_comm->testConnection();

	//would normally send query command here
	//QByteArray resp = p_comm->queryCmd(QString("*IDN?\r\n"));
	//parse resp... check for success
	GpibInstrument *g = static_cast<GpibInstrument*>(p_comm);
	emit logMessage(QString("GPIB address: %1").arg(g->address()));

	if(success)
	{
        readTripParameters();
		readFrequency();
		emit connected();
		return true;
	}
	else
	{
		emit connected(false,QString("GPIB error."));
		return false;
	}
}

void VirtualAomSynthesizer::initialize()
{
	testConnection();
}

Scan VirtualAomSynthesizer::prepareForScan(Scan scan)
{
	if(d_isActive)
    {
        readTripParameters();
		scan.addNumDataPoints(1);
    }

	return scan;
}

void VirtualAomSynthesizer::beginAcquisition()
{
}

void VirtualAomSynthesizer::endAcquisition()
{
}

void VirtualAomSynthesizer::readPointData()
{
	if(d_isActive)
	{
		QList<QPair<QString,QVariant>> out;
		out.append(qMakePair(QString("aom"),readFrequency()));
		//consider whether AOM frequency needs to be plotted?
		emit pointDataRead(out);
	}
}

void VirtualAomSynthesizer::setFrequency(double f)
{
	//would normally implement command here:
	//p_comm->writeCmd(QString("f=%1\r\n").arg(f));

	d_frequency = f;
	readFrequency();
}

double VirtualAomSynthesizer::readFrequency()
{
	//would normally do a query here
	//QByteArray resp = p_comm->queryCmd(QString("freq?\r\n"));
	//parse resp...
	emit frequencyUpdate(d_frequency);
	return d_frequency;
}

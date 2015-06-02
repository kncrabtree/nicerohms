#include "virtualioboard.h"

#include "virtualinstrument.h"

VirtualIOBoard::VirtualIOBoard(QObject *parent) :
	IOBoard(parent)
{
	d_subKey = QString("virtual");
	d_prettyName = QString("Virtual IO Board");
    d_isCritical = true;

	p_comm = new VirtualInstrument(d_key,this);
	connect(p_comm,&CommunicationProtocol::logMessage,this,&VirtualIOBoard::logMessage);
	connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

VirtualIOBoard::~VirtualIOBoard()
{

}



bool VirtualIOBoard::testConnection()
{
	p_lockReadTimer->stop();
	readIOSettings();
	readCavityLocked();

	p_lockReadTimer->start();
	emit connected();
	return true;
}

void VirtualIOBoard::initialize()
{
	testConnection();
}

Scan VirtualIOBoard::prepareForScan(Scan scan)
{
	readIOSettings();

	d_analogConfig = scan.ioboardAnalogConfig();
	d_digitalConfig = scan.ioboardDigitalConfig();

	int numPoints = 0;
	for(int i=0; i<d_analogConfig.size(); i++)
	{
		if(d_analogConfig.at(i).first)
			numPoints++;
	}
	for(int i=0; i<d_digitalConfig.size(); i++)
	{
		if(d_digitalConfig.at(i).second)
			numPoints++;
	}
	scan.addNumDataPoints(numPoints);

	return scan;
}

void VirtualIOBoard::beginAcquisition()
{
}

void VirtualIOBoard::endAcquisition()
{
}

void VirtualIOBoard::readPointData()
{
	QList<QPair<QString,QVariant>> analog;
	QList<QPair<QString,QVariant>> digital;

	for(int i=0; i<d_analogConfig.size(); i++)
	{
		if(d_analogConfig.at(i).first)
		{
			double r = static_cast<double>((qrand()%20000)-10000)/10000.0;
			switch (d_analogConfig.at(i).second) {
			case NicerOhms::LJR10V:
				analog.append(qMakePair(QString("ain%1").arg(i),r*10.0));
				break;
			case NicerOhms::LJR1V:
				analog.append(qMakePair(QString("ain%1").arg(i),r));
				break;
			case NicerOhms::LJR100mV:
				analog.append(qMakePair(QString("ain%1").arg(i),r*0.1));
				break;
			case NicerOhms::LJR10mV:
				analog.append(qMakePair(QString("ain%1").arg(i),r*0.01));
				break;
			default:
				break;
			}
		}
	}

	for(int i=0; i<d_digitalConfig.size(); i++)
	{
		if(d_digitalConfig.at(i).second)
		{
			bool b = qrand() % 2;
			digital.append(qMakePair(QString("din%1").arg(d_digitalConfig.at(i).first),b));
		}
	}

	emit pointDataRead(analog);
	emit pointDataReadNoPlot(digital);
}

bool VirtualIOBoard::readCavityLocked()
{
	bool on = qrand() % 1000;
	emit lockState(on);
	return on;
}

void VirtualIOBoard::setCavityPZTVoltage(double v)
{
	d_lastCavityVoltage = v;
}

void VirtualIOBoard::setCavityLockOverride(bool unlock)
{
	Q_UNUSED(unlock)
}

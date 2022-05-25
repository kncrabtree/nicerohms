#include "virtualioboard.h"

#include <hardware/core/communication/virtualinstrument.h>
#include <QRandomGenerator>

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
    //ioboard is always active
	readIOSettings();

	d_analogConfig = scan.ioboardAnalogConfig();
	d_digitalConfig = scan.ioboardDigitalConfig();

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    QString prefix = QString("IoBoard");

	int numPoints = 0;
	for(int i=0; i<d_analogConfig.size(); i++)
	{
		if(d_analogConfig.at(i).first)
        {
			numPoints++;
            QString name = s.value(QString("analogNames/ain%1").arg(i),QString("")).toString();
            QString r, units;
            switch(d_analogConfig.at(i).second)
            {
            case NicerOhms::LJR10V:
                r = QString("10");
                units = QString("V");
                break;
            case NicerOhms::LJR1V:
                r = QString("1");
                units = QString("V");
                break;
            case NicerOhms::LJR100mV:
                r = QString("100");
                units = QString("mV");
                break;
            case NicerOhms::LJR10mV:
                r = QString("10");
                units = QString("mV");
                break;
            }
            scan.addHeaderItem(prefix+QString("AIN%1Name").arg(i),name,QString(""));
            scan.addHeaderItem(prefix+QString("AIN%1Range").arg(i),r,QString(""));
        }
	}
	for(int i=0; i<d_digitalConfig.size(); i++)
	{
		if(d_digitalConfig.at(i).second)
        {
			numPoints++;
            int reserved = s.value(QString("%1/%2/reservedDigitalChannels").arg(d_key).arg(d_subKey),2).toInt();
            QString name = s.value(QString("digitalNames/din%1").arg(i+reserved)).toString();
            scan.addHeaderItem(prefix+QString("DIN%1Name").arg(i+reserved),name);
        }
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
    auto rg = QRandomGenerator::global();

	for(int i=0; i<d_analogConfig.size(); i++)
	{
		if(d_analogConfig.at(i).first)
		{
            double r = rg->generateDouble()*2.0-1.0;
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
            bool b = rg->bounded(1);
			digital.append(qMakePair(QString("din%1").arg(d_digitalConfig.at(i).first),b));
		}
	}

	emit pointDataRead(analog);
	emit pointDataReadNoPlot(digital);
}

bool VirtualIOBoard::readCavityLocked()
{
    auto r = QRandomGenerator::global();
    bool on = r->bounded(100);
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

void VirtualIOBoard::relock()
{
    emit relockComplete(true);
}



void VirtualIOBoard::relockPump()
{
}

void VirtualIOBoard::holdIntegrator(bool hold)
{
    Q_UNUSED(hold)
}

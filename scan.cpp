#include "scan.h"

#include <QSettings>
#include <QApplication>

Scan::Scan() : data(new ScanData)
{

}

Scan::Scan(const Scan &rhs) : data(rhs.data)
{

}

Scan &Scan::operator=(const Scan &rhs)
{
	if (this != &rhs)
		data.operator=(rhs.data);
	return *this;
}

Scan::~Scan()
{

}

bool Scan::isInitialized() const
{
	return data->isInitialized;
}

bool Scan::hardwareSuccess() const
{
	return data->hardwareSuccess;
}

bool Scan::isComplete() const
{
	return data->completedPoints >= totalPoints();
}

QString Scan::errorString() const
{
	return data->errorString;
}

QString Scan::startString() const
{
	return QString("Scan %1 starting.").arg(data->number);
}

int Scan::totalPoints() const
{
	return 100;
}

int Scan::completedPoints() const
{
	return data->completedPoints;
}

double Scan::currentLaserPos() const
{
	return 10.0;
}

bool Scan::isAutoLockEnabled() const
{
	return data->autoLockEnabled;
}

bool Scan::isAbortOnUnlock() const
{
	return data->abortOnUnlock;
}

bool Scan::laserDelay() const
{
	return data->laserDelay;
}

QPair<double, double> Scan::cavityPZTRange() const
{
	return qMakePair(data->cavityMin,data->cavityMax);
}

void Scan::setHardwareFailed()
{
	data->hardwareSuccess = false;
}

void Scan::setInitialized()
{
	data->isInitialized = true;

	data->startTime = QDateTime::currentDateTime();

	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	int num = s.value(QString("scanNum"),1).toInt();
	data->number = num;
}

void Scan::setAborted()
{
	data->aborted = true;
}

void Scan::setErrorString(const QString s)
{
	data->errorString = s;
}

bool Scan::validateData(const QList<QPair<QString, QVariant> > l)
{
	//can check data and see if scan should be automatically aborted
	//return false if the scan should abort
	//example: abort if pressure is below 2.0
	if(data->abortConditions.isEmpty())
		return true;

	//I'm assuming that all QVariants can be converted to doubles
	for(int i=0; i<l.size(); i++)
	{
		if(!l.at(i).second.canConvert(QVariant::Double))
			continue;

		QString key = l.at(i).first;
		bool ok = false;
		double value = l.at(i).second.toDouble(&ok);

		if(!ok)
			continue;

		if(data->abortConditions.contains(key))
		{
			auto range = data->abortConditions.value(key);
			if(value < range.first || value > range.second)
			{
				data->errorString = QString("Aborting because %1 is outside specified range (Value = %2, Min = %3, Max = %4")
						.arg(key).arg(value,0,'g',3).arg(range.first,0,'g',3).arg(range.second,0,'g',3);
				return false;
			}
		}
	}

	return true;
}

bool Scan::addPointData(const QList<QPair<QString, QVariant> > l)
{
	Q_UNUSED(l)
	//need to figure out whether all data are in
	return true;
}


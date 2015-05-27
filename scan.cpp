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

bool Scan::isAborted() const
{
	return data->aborted;
}

int Scan::number() const
{
	return data->number;
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
	return data->totalPoints;
}

int Scan::completedPoints() const
{
	return data->completedPoints;
}

double Scan::currentLaserPos() const
{
	double pos = data->laserStart + static_cast<double>(data->completedPoints)*data->laserStep;
	if(data->laserStep > 0)
		pos = qMin(pos,data->laserStop);
	else
		pos = qMax(pos,data->laserStop);

	return pos;
}

bool Scan::isAutoLockEnabled() const
{
	return data->autoLockEnabled;
}

bool Scan::isAbortOnUnlock() const
{
	return data->abortOnUnlock;
}

int Scan::laserDelay() const
{
	return data->laserDelay;
}

QPair<double, double> Scan::cavityPZTRange() const
{
	return qMakePair(data->cavityMin,data->cavityMax);
}

QString Scan::endLogMessage() const
{
	return data->endLogMessage;
}

NicerOhms::LogMessageCode Scan::endLogCode() const
{
	return data->endLogCode;
}

bool Scan::isHardwareActive(QString key)
{
	//if key is not found, assume hardware is inactive
	return data->activeHardware.value(key,false);
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

	data->endLogMessage = QString("Scan %1 complete.").arg(data->number);
	data->endLogCode = NicerOhms::LogHighlight;
}

void Scan::setAborted()
{
	data->endLogMessage = QString("Scan %1 aborted.").arg(data->number);
	data->endLogCode = NicerOhms::LogError;
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
	data->dataCache.append(l);

	if(data->dataCache.size() == data->numDataPoints)
	{
		if(!data->redo)
			saveData();
		data->redo = false;
		data->dataCache.clear();
		return true;
	}

	return false;


}

void Scan::addNumDataPoints(int n)
{
	data->numDataPoints += n;
}

void Scan::setPointRedo()
{
	data->redo = true;
}

void Scan::setLaserParams(double start, double stop, double step, int delay)
{
	data->laserStart = start;
	data->laserStop = stop;
	data->laserDelay = delay;

	if(stop > start)
		data->laserStep = fabs(step);
	else
		data->laserStep = -fabs(step);

	int points = static_cast<int>(floor(fabs((data->laserStart - data->laserStop)/data->laserStep))) + 2;
	if(qFuzzyCompare(1.0 + data->laserStart + static_cast<double>(points - 1)*data->laserStep,1.0 + data->laserStop + data->laserStep))
		points -= 1;

	data->totalPoints = points;
}

void Scan::addHardwareItem(QString key, bool active)
{
	data->activeHardware.insert(key,active);
}

void Scan::saveData()
{
	for(int i=0; i<data->dataCache.size(); i++)
	{
		QString key = data->dataCache.at(i).first;
		QVariant value = data->dataCache.at(i).second;

		if(data->scanData.contains(key))
			data->scanData[key].append(value);
		else
		{
			QVector<QVariant> d;
			d.reserve(totalPoints());
			d.append(value);
			data->scanData.insert(key,d);
		}
	}

	//iterate over data->scanData and write items to disk

	data->completedPoints++;

}


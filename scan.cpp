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

QString Scan::headerString() const
{
	QString out;

	auto it = data->headerData.constBegin();
	while(it != data->headerData.constEnd())
	{
		QString key = it.key();
		QString val = it.value().first;
		QString unit = it.value().second;

		out.append(QString("#%1\t%2\t%3\n").arg(key).arg(val).arg(unit));
	}

	return out;
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

	addHeaderItem(QString("ScanNumber"),num);
	addHeaderItem(QString("ScanStartTime"),data->startTime);
	addHeaderItem(QString("ScanTotalPoints"),data->totalPoints);
	addHeaderItem(QString("ScanAutoLockEnabled"),data->autoLockEnabled);
	if(data->autoLockEnabled)
	{
		addHeaderItem(QString("ScanAutoLockMin"),data->cavityMin,QString("V"));
		addHeaderItem(QString("ScanAutoLockMax"),data->cavityMax,QString("V"));
	}
	addHeaderItem(QString("ScanNumDataPoints"),data->numDataPoints);
	addHeaderItem(QString("ScanPointDelay"),data->laserDelay,QString("ms"));
	addHeaderItem(QString("ScanAbortOnUnlock"),data->abortOnUnlock);

	//figure out how to include start, stop, and step
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

void Scan::addHeaderItem(QString key, QVariant value, QString units)
{
	data->headerData.insert(key,qMakePair(value.toString(),units));
}

Scan::PointAction Scan::validateData(const QList<QPair<QString, QVariant> > l)
{
	//can check data and see if scan should be automatically aborted
	//return false if the scan should abort
	//example: abort if pressure is below 2.0
    PointAction out = Continue;
    if(data->validationConditions.isEmpty())
        return out;

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

        if(data->validationConditions.contains(key))
		{
            QList<PointValidation> validationList = data->validationConditions.values(key);
            for(int i=0; i<validationList.size(); i++)
            {
                const PointValidation &v = validationList.at(i);
                if(value < v.min || value > v.max)
                {
                    if(v.action == Abort)
                    {
                        data->errorString = QString("Aborting because %1 is outside specified range (Value = %2, Min = %3, Max = %4).")
                                .arg(key).arg(value,0,'g',v.precision).arg(v.min,0,'g',v.precision).arg(v.max,0,'g',v.precision);
                        return Abort;
                    }
                    else if(v.action == Remeasure)
                    {
                        data->errorString = QString("Remeasuring because %1 is outside specified range (Value = %2, Min = %3, Max = %4).")
                                .arg(key).arg(value,0,'g',v.precision).arg(v.min,0,'g',v.precision).arg(v.max,0,'g',v.precision);
                        out = Remeasure;
                    }
                }
			}
		}
	}

    return out;
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

void Scan::addValidationItem(QString key, double min, double max, Scan::PointAction action, int precision)
{
    PointValidation v;
    v.min = min;
    v.max = max;
    v.action = action;
    v.precision = precision;

    if(data->validationConditions.contains(key))
        data->validationConditions.insertMulti(key,v);
    else
        data->validationConditions.insert(key,v);

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


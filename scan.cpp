#include "scan.h"
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

Scan::Scan(ScanType t) : data(new ScanData)
{
	data->type = t;
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

Scan::ScanType Scan::type() const
{
	return data->type;
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

double Scan::currentPos() const
{
	double pos = data->scanStart + static_cast<double>(data->completedPoints)*data->scanStep;
	if(data->scanStep > 0)
		pos = qMin(pos,data->scanStop);
	else
		pos = qMax(pos,data->scanStop);

	return pos;
}

double Scan::combShift() const
{
	if(data->completedPoints == 0)
		return 0.0;
	else
	{
		double lastPos = data->scanStart + static_cast<double>(data->completedPoints-1)*data->scanStep;
		return currentPos() - lastPos;
	}

}

bool Scan::isAutoLockEnabled() const
{
	return data->autoLockEnabled;
}

bool Scan::isAbortOnUnlock() const
{
	return data->abortOnUnlock;
}

int Scan::delay() const
{
	return data->scanDelay;
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

bool Scan::isHardwareActive(QString key) const
{
	//if key is not found, assume hardware is inactive
	return data->activeHardware.value(key,false);
}

QList<QPair<bool, NicerOhms::LabJackRange> > Scan::ioboardAnalogConfig() const
{
	return data->ioboardAnalogConfig;
}

QList<QPair<int, bool> > Scan::ioboardDigitalConfig() const
{
	return data->ioboardDigitalConfig;
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
	addHeaderItem(QString("ScanPointDelay"),data->scanDelay,QString("ms"));
	addHeaderItem(QString("ScanAbortOnUnlock"),data->abortOnUnlock);

    if(data->type == Scan::LaserScan)
    {
        addHeaderItem(QString("ScanStartPos"),QString::number(data->scanStart,'f',3),QString("V"));
        addHeaderItem(QString("ScanStopPos"),QString::number(data->scanStop,'f',3),QString("V"));
        addHeaderItem(QString("ScanStepSize"),QString::number(data->scanStep,'f',3),QString("V"));
    }
    else
    {
        addHeaderItem(QString("ScanStartPos"),QString::number(data->scanStart,'f',3),QString("MHz"));
        addHeaderItem(QString("ScanStopPos"),QString::number(data->scanStop,'f',3),QString("MHz"));
        addHeaderItem(QString("ScanStepSize"),QString::number(data->scanStep,'f',3),QString("MHz"));
    }


    //get file
    int millions = data->number/1000000;
    int thousands = data->number/1000;
    QString dirName = QString("%1/scans/%2/%3").arg(s.value(QString("savePath"),QString(".")).toString()).arg(millions).arg(thousands);

    QDir d(dirName);
    if(!d.exists())
    {
        if(!d.mkpath(d.absolutePath()))
        {
            //this is bad... abort!
            data->isInitialized = false;
            data->errorString = QString("Could not create directory %1 for saving data.").arg(dirName);
            return;
        }
    }

    //create output file
    QFile f(QString("%1/%2.txt").arg(d.absolutePath()).arg(data->number));
    if(f.exists())
    {
        //this is bad... abort!
        data->isInitialized = false;
        data->errorString = QString("The file %1 already exists! Make sure that the value of scanNum in the settings file is one greater than the last completed scan.").arg(f.fileName());
        return;
    }

    if(!f.open(QIODevice::WriteOnly))
    {
        //this is also bad... abort!
        data->isInitialized = false;
        data->errorString = QString("The file %1 could not be opened for writing.").arg(f.fileName());
        return;
    }

    data->fileName = f.fileName();
    QTextStream t(&f);

    QString hash = QString("#");
    QString tab = QString("\t");
    QString nl = QString("\n");
    //write header
    auto it = data->headerData.constBegin();
    while(it != data->headerData.constEnd())
    {
        t << hash << it.key() << tab << it.value().first << tab << it.value().second << nl;
        it++;
    }
    //comments
    QStringList l = data->comments.split(QString("\n"));
    int lines = l.size();
    t << hash << QString("CommentLines") << tab << lines << tab << nl;
    for(int i=0; i<lines; i++)
        t << hash << l.at(i) << nl;
    t << nl;

    f.close();

    s.setValue(QString("scanNum"),data->number+1);
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
			finishPoint();
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

void Scan::setScanParams(double start, double stop, double step, int delay)
{
	data->scanStart = start;
	data->scanStop = stop;
	data->scanDelay = delay;

	if(stop > start)
		data->scanStep = fabs(step);
	else
		data->scanStep = -fabs(step);

	int points = static_cast<int>(floor(fabs((data->scanStart - data->scanStop)/data->scanStep))) + 2;
	if(qFuzzyCompare(1.0 + data->scanStart + static_cast<double>(points - 1)*data->scanStep,1.0 + data->scanStop + data->scanStep))
		points -= 1;

	data->totalPoints = points;
}

void Scan::addHardwareItem(QString key, bool active)
{
	data->activeHardware.insert(key,active);
}

void Scan::setCavityPZTRange(double min, double max)
{
	data->cavityMin = min;
	data->cavityMax = max;
}

void Scan::setAutoRelock(bool enabled)
{
	data->autoLockEnabled = enabled;
}

void Scan::setAbortOnUnlock(bool abort)
{
	data->abortOnUnlock = abort;
}

void Scan::setComments(QString c)
{
	data->comments = c;
}

void Scan::finalSave()
{
	//will do some disk writing eventually
	//save keys for use in completer
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	QString keys = s.value(QString("knownValidationKeys"),QString("")).toString();
	QStringList knownKeyList = keys.split(QChar(';'),QString::SkipEmptyParts);

	auto it = data->scanData.constBegin();
	while(it != data->scanData.constEnd())
	{
		QString key = it.key();
		if(!knownKeyList.contains(key))
			knownKeyList.append(key);
		it++;
	}

	keys.clear();
	if(knownKeyList.size() > 0)
	{
		keys = knownKeyList.at(0);
		for(int i=1; i<knownKeyList.size();i++)
			keys.append(QString(";%1").arg(knownKeyList.at(i)));

		s.setValue(QString("knownValidationKeys"),keys);
	}

	//save to disk...
}

void Scan::setIOBoardAnalog(QList<QPair<bool, NicerOhms::LabJackRange> > l)
{
	data->ioboardAnalogConfig = l;
}

void Scan::setIOBoardDigital(QList<QPair<int, bool> > l)
{
	data->ioboardDigitalConfig = l;
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

void Scan::finishPoint()
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

    QFile f(data->fileName);

    if(f.open(QIODevice::Append))
    {
        QTextStream t(&f);
        QString tab("\t");
        QString nl("\n");

        auto it = data->scanData.constBegin();
        //iterate over data->scanData and write items to disk
        if(data->completedPoints == 0)
        {
            //need to write headers
            while(it + 1 != data->scanData.constEnd())
            {
                t << QString("%1_%2").arg(it.key()).arg(data->number) << tab;
                it++;
            }
            t << QString("%1_%2").arg(it.key()).arg(data->number) << nl;
            it = data->scanData.constBegin();
        }

        //write data
        t.setRealNumberPrecision(14);
        t.setRealNumberNotation(QTextStream::SmartNotation);
        while(it + 1 != data->scanData.constEnd())
        {
            QVector<QVariant> vec = data->scanData.value(it.key());
            t << vec.at(vec.size()-1).toDouble() << tab;
            it++;
        }
        QVector<QVariant> vec = data->scanData.value(it.key());
        t << vec.at(vec.size()-1).toDouble() << nl; //noticed voltage rounds to first decimal
        f.close();
    }

	data->completedPoints++;

}


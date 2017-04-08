#include "freqcombdata.h"
#include <QDebug>
#include <QSettings>
#include <QApplication>
FreqCombData::FreqCombData() : data(new FreqCombDataData)
{

}

FreqCombData::FreqCombData(const FreqCombData &rhs) : data(rhs.data)
{

}

FreqCombData &FreqCombData::operator=(const FreqCombData &rhs)
{
	if (this != &rhs)
		data.operator=(rhs.data);
	return *this;
}

FreqCombData::~FreqCombData()
{

}

bool FreqCombData::success() const
{
	return data->success;
}

int FreqCombData::faultCode() const
{
	return data->faultCode;
}

QString FreqCombData::faultMsg() const
{
	return data->faultMsg;
}

double FreqCombData::repRate() const
{
	return data->repRate;
}

double FreqCombData::offsetBeat() const
{
	return data->offsetBeat;
}

double FreqCombData::pumpBeat() const
{
	return data->pumpBeat;
}

double FreqCombData::signalBeat() const
{
	return data->signalBeat;
}

double FreqCombData::aomFreq() const
{
    return data->aomFreq;
}

double FreqCombData::counterFreq() const
{
    return data->counterFreq;
}

int FreqCombData::deltaN() const
{
	return data->deltaN;
}

double FreqCombData::repRateLockVoltage() const
{
	return data->repRateLockVoltage;
}

bool FreqCombData::repRateLocked() const
{
	return data->repRateLocked;
}

double FreqCombData::calculatedIdlerFreq() const
{
    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    bool sigLock = set.value(QString("lastScanConfig/signalLock"),false).toBool();
    bool pumpLock = set.value(QString("lastScanConfig/pumpLock"),false).toBool();


    if(sigLock)
    {
        if(pumpLock)
        {
            return data->repRate*static_cast<double>(data->deltaN) + (data->pumpBeat - data->signalBeat) - 2.0*data->counterFreq;

        }
        else
        {
            return data->repRate*static_cast<double>(data->deltaN) + (data->pumpBeat - data->signalBeat) - 2.0*data->aomFreq*1e6;

        }


    //Switched sign on AOM frequency to account for pump shift.

    }
    else
    {
    return data->repRate*static_cast<double>(data->deltaN) + (data->pumpBeat - data->signalBeat) + 2.0*data->aomFreq*1e6;
    }


}

void FreqCombData::parseXml(const QDomDocument &d)
{
    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    QString sk = set.value(QString("freqComb/subKey"),QString("virtual")).toString();

    bool counterMode = set.value(QString("freqComb/%1/counterConfig").arg(sk),false).toBool();


	QDomElement root = d.documentElement();
	if(root.firstChild().nodeName() == QString("fault"))
	{
		data->success = false;
		//navigate to faultcode
		QDomNode fstructnode = root.firstChild().firstChild().firstChild();
		data->faultCode = fstructnode.firstChild().firstChild().nextSibling().firstChildElement(QString("i4")).text().toInt();
		data->faultMsg = fstructnode.firstChild().nextSibling().firstChild().nextSiblingElement(QString("value")).text();
		return;
	}

	//navigate through DOM tree to the first "member" tag
	QDomElement m1 = root.firstChild().firstChild().firstChild().firstChild().firstChildElement(QString("member"));
	//now go to the "struct" tag, and get a list of its members
	QDomNodeList l = m1.firstChild().nextSibling().firstChildElement(QString("struct")).elementsByTagName(QString("member"));
	//loop over the list of members, extracting the desired data
	for(int i=0;i<l.size();i++)
	{
		QDomNode el = l.at(i);
		QVariant val = el.firstChildElement(QString("value")).text();
		QString dataName = el.firstChildElement(QString("name")).text();
		if(dataName == QString("offset.freq"))
			data->offsetBeat = val.toDouble();
		else if(dataName == QString("reprate.freq"))
			data->repRate = val.toDouble();
        else if(dataName == QString("counter2.freq"))//counter 2 no internal bandpass
        {
            if(counterMode)
                data->signalBeat = val.toDouble();
            else
                data->pumpBeat = val.toDouble();
        }
        else if(dataName == QString("counter3.freq"))//counter 3 has internal bandpass
        {
            if(counterMode)
                data->pumpBeat = val.toDouble();
            else
                data->signalBeat = val.toDouble();
        }
		else if(dataName == QString("lb1.mon"))
			data->repRateLockVoltage = val.toDouble();
		else if(dataName == QString("lb1.status"))
		{
			int lockStatus = val.toInt();
			if(lockStatus == 2)
				data->repRateLocked = true;
			else
				data->repRateLocked = false;
		}
	}
}

void FreqCombData::setBeatSigns(bool pumpPositive, bool signalPositive)
{
	if(pumpPositive)
		data->pumpBeat = fabs(data->pumpBeat);
	else
		data->pumpBeat = -fabs(data->pumpBeat);

	if(signalPositive)
		data->signalBeat = fabs(data->signalBeat);
	else
		data->signalBeat = -fabs(data->signalBeat);
}

int FreqCombData::setDeltaN(double idlerFreq, double aomFreq, double counterFreq)
{

    //add counter calc
	if(data->repRate < 1.0)
        return 0.0;

	data->aomFreq = aomFreq;
    data->counterFreq = counterFreq;

    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    bool sigLock = set.value(QString("lastScanConfig/signalLock"),false).toBool();
    bool pumpLock = set.value(QString("lastScanConfig/pumpLock"),false).toBool();
	//idlerfreq is in Hz; so is rep rate
    if(sigLock)
    {
        if(pumpLock)
        {
            data->deltaN = qRound((idlerFreq - data->pumpBeat + data->signalBeat + 2.0*counterFreq)/data->repRate);
        }//changed to +pumpbeat - signalbeat + 2 counter
        else
        {
            data->deltaN = qRound((idlerFreq - data->pumpBeat + data->signalBeat + 2.0*aomFreq*1e6)/data->repRate);//check units aomfreq...changed to be in Hz
        }//changed to +pumpbeat - signalbeat + 2 aom
    }
    else
    {
        data->deltaN = qRound((idlerFreq - data->pumpBeat + data->signalBeat - 2.0*aomFreq*1e6)/data->repRate);
    }//changed to +pump - signal -2aom
    return data->deltaN;
}

void FreqCombData::setDeltaN(int dN, double aomFreq, double counterFreq)
{
	data->deltaN = dN;
    data->aomFreq = aomFreq;
    data->counterFreq = counterFreq;
}

void FreqCombData::setSuccess(bool b)
{
    data->success = b;
}

void FreqCombData::setRepRate(double r)
{
	data->repRate = r;
}

void FreqCombData::setOffsetBeat(double b)
{
	data->offsetBeat = b;
}

void FreqCombData::setPumpBeat(double b)
{
	data->pumpBeat = b;
}

void FreqCombData::setSignalBeat(double b)
{
	data->signalBeat = b;
}

void FreqCombData::setRepRateLockVoltage(double v)
{
	data->repRateLockVoltage = v;
}

void FreqCombData::setRepRateLocked(bool locked)
{
	data->repRateLocked = locked;
}


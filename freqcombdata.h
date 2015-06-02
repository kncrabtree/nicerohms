#ifndef FREQCOMBDATA_H
#define FREQCOMBDATA_H

#include <QSharedDataPointer>

#include <QtXml/QDomDocument>
#include <QVariant>

class FreqCombDataData;

class FreqCombData
{
public:
	FreqCombData();
	FreqCombData(const FreqCombData &);
	FreqCombData &operator=(const FreqCombData &);
	~FreqCombData();

	bool success() const;
	int faultCode() const;
	QString faultMsg() const;
	double repRate() const;
	double offsetBeat() const;
	double pumpBeat() const;
	double signalBeat() const;
	double aomFreq() const;
	int deltaN() const;
	double repRateLockVoltage() const;
	bool repRateLocked() const;
	double calculatedIdlerFreq() const;

	void parseXml(const QDomDocument &d);
	void setBeatSigns(bool pumpPositive, bool signalPositive);
	void setDeltaN(double idlerFreq, double aomFreq);
	void setDeltaN(int dN, double aomFreq);

	//the following functions are not recommended for use outside a virtual implementation!
    void setSuccess(bool b);
	void setRepRate(double r);
	void setOffsetBeat(double b);
	void setPumpBeat(double b);
	void setSignalBeat(double b);
	void setRepRateLockVoltage(double v);
	void setRepRateLocked(bool locked);

private:
	QSharedDataPointer<FreqCombDataData> data;
};


class FreqCombDataData : public QSharedData
{
public:
    FreqCombDataData() : success(true), faultCode(0), repRate(-1.0), offsetBeat(0.0), pumpBeat(0.0), signalBeat(0.0), aomFreq(0.0),
		deltaN(-1), repRateLockVoltage(0.0), repRateLocked(false) {}

	bool success;
	int faultCode;
	QString faultMsg;
	double repRate;
	double offsetBeat;
	double pumpBeat;
	double signalBeat;
	double aomFreq;
	int deltaN;
	double repRateLockVoltage;
	bool repRateLocked;


};

#endif // FREQCOMBDATA_H

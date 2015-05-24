#ifndef SCAN_H
#define SCAN_H

#include <QSharedDataPointer>

#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QVector>

#include "datastructs.h"

class ScanData;

class Scan
{
public:
	Scan();
	Scan(const Scan &);
	Scan &operator=(const Scan &);
	~Scan();

	bool isInitialized() const;
	bool hardwareSuccess() const;
	bool isComplete() const;
	bool isAborted() const;
	QString errorString() const;
	QString startString() const;
	int totalPoints() const;
	int completedPoints() const;
	double currentLaserPos() const;
	bool isAutoLockEnabled() const;
	bool isAbortOnUnlock() const;
	bool laserDelay() const;
	QPair<double,double> cavityPZTRange() const;
	QString endLogMessage() const;
	NicerOhms::LogMessageCode endLogCode() const;


	void setHardwareFailed();
	void setInitialized();
	void setAborted();
	void setErrorString(const QString s);
	bool validateData(const QList<QPair<QString,QVariant>> l);
	bool addPointData(const QList<QPair<QString,QVariant>> l);
	void addNumDataPoints(int n);
	void setPointRedo();

private:
	QSharedDataPointer<ScanData> data;

	void saveData();
};


class ScanData : public QSharedData
{
public:
	ScanData() : number(0), isInitialized(false), hardwareSuccess(true), aborted(false), completedPoints(0), autoLockEnabled(false),
	cavityMin(0.0), cavityMax(150.0), abortOnUnlock(false), laserDelay(0), numDataPoints(0), redo(false) {}

	int number;
	bool isInitialized;
	bool hardwareSuccess;
	bool aborted;
	QDateTime startTime;
	int completedPoints;
	bool autoLockEnabled;
	double cavityMin, cavityMax;
	bool abortOnUnlock;
	int laserDelay;
	int numDataPoints;
	bool redo;

	QList<QPair<QString,QVariant>> dataCache;
	QMap<QString,QVector<QVariant>> scanData;
	QString errorString;
	QString endLogMessage;
	NicerOhms::LogMessageCode endLogCode;
	QMap<QString,QPair<double,double>> abortConditions;
};

#endif // SCAN_H

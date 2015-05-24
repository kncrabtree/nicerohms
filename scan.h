#ifndef SCAN_H
#define SCAN_H

#include <QSharedDataPointer>

#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>
#include <QVariant>

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
	QString errorString() const;
	QString startString() const;
	int totalPoints() const;
	int completedPoints() const;
	double currentLaserPos() const;
	bool isAutoLockEnabled() const;
	bool isAbortOnUnlock() const;


	void setHardwareFailed();
	void setInitialized();
	void setAborted();
	void setErrorString(const QString s);
	bool validateData(const QList<QPair<QString,QVariant>> l);
	bool addPointData(const QList<QPair<QString,QVariant>> l);

private:
	QSharedDataPointer<ScanData> data;
};


class ScanData : public QSharedData
{
public:
	ScanData() : number(0), isInitialized(false), hardwareSuccess(true), aborted(false), completedPoints(0), autoLockEnabled(false),
	abortOnUnlock(false) {}

	int number;
	bool isInitialized;
	bool hardwareSuccess;
	bool aborted;
	QDateTime startTime;
	int completedPoints;
	bool autoLockEnabled;
	bool abortOnUnlock;

	QString errorString;
	QMap<QString,QPair<double,double>> abortConditions;
};

#endif // SCAN_H

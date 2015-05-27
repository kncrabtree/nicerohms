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

    enum PointAction {
        Continue,
        Remeasure,
        Abort
    };

    struct PointValidation
    {
        PointValidation() : min(0.0), max(1.0), action(Scan::Continue), precision(3) {}

        double min;
        double max;
        Scan::PointAction action;
        int precision;
    };

	bool isInitialized() const;
	bool hardwareSuccess() const;
	bool isComplete() const;
	bool isAborted() const;
	int number() const;
	QString errorString() const;
	QString startString() const;
	int totalPoints() const;
	int completedPoints() const;
	double currentLaserPos() const;
	bool isAutoLockEnabled() const;
	bool isAbortOnUnlock() const;
	int laserDelay() const;
	QPair<double,double> cavityPZTRange() const;
	QString endLogMessage() const;
	NicerOhms::LogMessageCode endLogCode() const;
	bool isHardwareActive(QString key);


	void setHardwareFailed();
	void setInitialized();
	void setAborted();
	void setErrorString(const QString s);
    PointAction validateData(const QList<QPair<QString,QVariant>> l);
	bool addPointData(const QList<QPair<QString,QVariant>> l);
	void addNumDataPoints(int n);
	void setPointRedo();
	void setLaserParams(double start, double stop, double step, int delay);
	void addHardwareItem(QString key, bool active);

    //multiple entries with same key are OK; Abort takes precedence over Redo, which in turn takes precedence over Continue
    void addValidationItem(QString key, double min, double max, Scan::PointAction action, int precision = 3);

private:
	QSharedDataPointer<ScanData> data;

	void saveData();
};


class ScanData : public QSharedData
{
public:
	ScanData() : number(0), isInitialized(false), hardwareSuccess(true), aborted(false), completedPoints(0), totalPoints(0),
		autoLockEnabled(false), cavityMin(0.0), cavityMax(150.0), abortOnUnlock(false), laserDelay(0), numDataPoints(0), redo(false),
		laserStart(0.0), laserStop(0.0), laserStep(0.0) {}

	int number;
	bool isInitialized;
	bool hardwareSuccess;
	bool aborted;
	QDateTime startTime;
	int completedPoints;
	int totalPoints;
	bool autoLockEnabled;
	double cavityMin, cavityMax;
	bool abortOnUnlock;
	int laserDelay;
	int numDataPoints;
	bool redo;
	double laserStart;
	double laserStop;
	double laserStep;

	QList<QPair<QString,QVariant>> dataCache;
	QMap<QString,bool> activeHardware;
	QMap<QString,QVector<QVariant>> scanData;
	QString errorString;
	QString endLogMessage;
	NicerOhms::LogMessageCode endLogCode;
    QMap<QString,Scan::PointValidation> validationConditions;
};

#endif // SCAN_H

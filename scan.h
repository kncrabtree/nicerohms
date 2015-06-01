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
	enum ScanType {
		LaserScan,
		CombScan
	};

	Scan(ScanType t = LaserScan);
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

    ScanType type() const;
	bool isInitialized() const;
	bool hardwareSuccess() const;
	bool isComplete() const;
	bool isAborted() const;
	int number() const;
	QString errorString() const;
	QString startString() const;
	QString headerString() const;
	int totalPoints() const;
	int completedPoints() const;
	double currentPos() const;
	bool isAutoLockEnabled() const;
	bool isAbortOnUnlock() const;
	int delay() const;
	QPair<double,double> cavityPZTRange() const;
	QString endLogMessage() const;
	NicerOhms::LogMessageCode endLogCode() const;
	bool isHardwareActive(QString key) const;


	void setHardwareFailed();
	void setInitialized();
	void setAborted();
	void setErrorString(const QString s);
	void addHeaderItem(QString key, QVariant value, QString units = QString(""));
	PointAction validateData(const QList<QPair<QString,QVariant>> l);
	bool addPointData(const QList<QPair<QString,QVariant>> l);
	void addNumDataPoints(int n);
	void setPointRedo();
	void setScanParams(double start, double stop, double step, int delay);
	void addHardwareItem(QString key, bool active = true);
	void setCavityPZTRange(double min, double max);
	void setAutoRelock(bool enabled);
	void setAbortOnUnlock(bool abort);
	void setComments(QString c);
	void finalSave();

    //multiple entries with same key are OK; Abort takes precedence over Redo, which in turn takes precedence over Continue
    void addValidationItem(QString key, double min, double max, Scan::PointAction action, int precision = 3);

private:
	QSharedDataPointer<ScanData> data;

	void finishPoint();
};


class ScanData : public QSharedData
{
public:
	ScanData() : type(Scan::LaserScan), number(0), isInitialized(false), hardwareSuccess(true), aborted(false), completedPoints(0),
		totalPoints(0), autoLockEnabled(false), cavityMin(0.0), cavityMax(150.0), abortOnUnlock(false), scanDelay(0),
		numDataPoints(0), redo(false), scanStart(0.0), scanStop(0.0), scanStep(0.0) {}

	Scan::ScanType type;
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
	int scanDelay;
	int numDataPoints;
	bool redo;
	double scanStart;
	double scanStop;
	double scanStep;
	QString comments;

	QList<QPair<QString,QVariant>> dataCache;
	QMap<QString,bool> activeHardware;
	QMap<QString,QVector<QVariant>> scanData;
	QMap<QString,QPair<QString,QString>> headerData;
	QString errorString;
	QString endLogMessage;
	NicerOhms::LogMessageCode endLogCode;
    QMap<QString,Scan::PointValidation> validationConditions;
};

#endif // SCAN_H

#ifndef ACQUISITIONMANAGER_H
#define ACQUISITIONMANAGER_H

#include <QObject>

#include "scan.h"
#include "datastructs.h"

class AcquisitionManager : public QObject
{
	Q_OBJECT
public:
	explicit AcquisitionManager(QObject *parent = 0);
	~AcquisitionManager();

	enum AcquisitionState {
		Idle,
		Acquiring,
		WaitingForFrequency,
		WaitingForLockCheck,
		WaitingForAutoLock,
		WaitingForManualLock,
		WaitingForRedo
	};

signals:
	void logMessage(const QString,const NicerOhms::LogMessageCode = NicerOhms::LogNormal);
	void statusMessage(const QString);
	void beginAcquisition();
	void scanComplete(Scan);
	void startLaserPoint(double);
	void startCombPoint(double);
    void checkLock(bool cPZT);
	void requestAutoLock();
	void requestManualLock();
	void getPointData();
	void pointComplete(int);
	void plotData(QList<QPair<QString,QVariant>>,double);

public slots:
	void initialize();
	void beginScan(Scan s);
	void processData(QList<QPair<QString,QVariant>> l, bool plot);
	void beginPoint();
	void frequencyReady();
	void lockCheckComplete(bool locked, double cavityVoltage);
	void autoLockComplete(bool success);
	void manualLockComplete(bool abort);
	void abortScan();
	void lockStateUpdate(bool locked);

private:
	Scan d_currentScan;
	AcquisitionState d_currentState;
	QList<QPair<QString,QVariant>> d_plotDataCache;

	void checkScanComplete();
	void endAcquisition();
};

#endif // ACQUISITIONMANAGER_H

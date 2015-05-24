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
		WaitingForLaser,
		WaitingForLockCheck,
		WaitingForAutoLock,
		WaitingForManualLock
	};

signals:
	void logMessage(const QString,const NicerOhms::LogMessageCode = NicerOhms::LogNormal);
	void statusMessage(const QString);
	void beginAcquisition();
	void scanComplete(Scan);
	void startPoint(double);
	void checkLock();
	void requestAutoLock();
	void requestManualLock();
	void getPointData();
	void pointComplete(int);

public slots:
	void initialize();
	void beginScan(Scan s);
	void processData(QList<QPair<QString,QVariant>> l);
	void beginPoint();
	void laserReady();
	void lockCheckComplete(bool locked, double cavityVoltage);
	void autoLockComplete(bool success);
	void manualLockComplete(bool abort);
	void abortScan();

private:
	Scan d_currentScan;
	AcquisitionState d_currentState;

	void checkScanComplete();
	void endAcquisition();
};

#endif // ACQUISITIONMANAGER_H

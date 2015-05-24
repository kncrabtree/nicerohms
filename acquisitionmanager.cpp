#include "acquisitionmanager.h"

AcquisitionManager::AcquisitionManager(QObject *parent) : QObject(parent), d_currentState(Idle)
{

}

AcquisitionManager::~AcquisitionManager()
{

}

void AcquisitionManager::initialize()
{

}

void AcquisitionManager::beginScan(Scan s)
{
	if(!s.isInitialized())
	{
		if(!s.errorString().isEmpty())
			emit logMessage(s.errorString(),NicerOhms::LogError);

		emit scanComplete(s);
	}

	d_currentScan = s;

	d_currentState = Acquiring;
	emit logMessage(s.startString(),NicerOhms::LogHighlight);
	emit beginAcquisition();

	beginPoint();
}

void AcquisitionManager::processData(QList<QPair<QString, QVariant> > l)
{
	if(d_currentState == Acquiring)
	{
		if(!d_currentScan.validateData(l))
			abortScan();
		else
		{
			//addPointData returns true if point is now complete
			if(d_currentScan.addPointData(l))
			{
				emit pointComplete(d_currentScan.completedPoints());
				checkScanComplete();
			}
		}
	}
}

void AcquisitionManager::beginPoint()
{
	if(d_currentState == Acquiring)
	{
		d_currentState = WaitingForLaser;
		emit startPoint(d_currentScan.currentLaserPos());
	}
}

void AcquisitionManager::laserReady(bool locked)
{
	if(d_currentState == WaitingForLaser)
	{
		if(locked)
		{
			d_currentState = Acquiring;
			emit getPointData();
		}
		else
		{
			if(d_currentScan.isAutoLockEnabled())
			{
				d_currentState = WaitingForAutoLock;
				emit requestAutoLock();
			}
			else
			{
				if(d_currentScan.isAbortOnUnlock())
					abortScan();
				else
				{
					d_currentState = WaitingForManualLock;
					emit requestManualLock();
				}
			}
		}
	}
}

void AcquisitionManager::autoLockComplete(bool success)
{
	if(d_currentState == WaitingForAutoLock)
	{
		if(success)
		{
			d_currentState = Acquiring;
			emit getPointData();
		}
		else
		{
			if(d_currentScan.isAbortOnUnlock())
				abortScan();
			else
			{
				d_currentState = WaitingForManualLock;
				emit requestManualLock();
			}
		}
	}
}

void AcquisitionManager::manualLockComplete(bool abort)
{
	if(abort)
		abortScan();
	else
	{
		d_currentState = Acquiring;
		emit getPointData();
	}
}

void AcquisitionManager::abortScan()
{
	if(d_currentState != Idle)
	{
		d_currentScan.setAborted();
		endAcquisition();
	}
}

void AcquisitionManager::checkScanComplete()
{
	if(d_currentScan.isComplete())
		endAcquisition();
	else
		beginPoint();
}

void AcquisitionManager::endAcquisition()
{
	d_currentState = Idle;
}


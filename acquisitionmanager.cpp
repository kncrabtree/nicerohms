#include "acquisitionmanager.h"
#include <QTimer>
#include <QDebug>

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
		emit scanComplete(s);
		return;
	}

	d_currentScan = s;

	d_currentState = Acquiring;
    if(s.type() == Scan::LaserScan)
        emit statusMessage(QString("Slewing laser to starting position"));

	emit logMessage(s.startString(),NicerOhms::LogHighlight);
	emit beginAcquisition();

	beginPoint();

}

void AcquisitionManager::processData(QList<QPair<QString, QVariant> > l, bool plot)
{
	if(d_currentState != Idle)
	{
		double x = d_currentScan.currentPos();
		if(plot)
			d_plotDataCache.append(l);

		Scan::PointAction a = d_currentScan.validateData(l);
		if(a == Scan::Abort)
			abortScan();
		else
		{
			if(a == Scan::Remeasure)
			{
				if(!d_currentScan.errorString().isEmpty())
				{
					emit logMessage(d_currentScan.errorString(),NicerOhms::LogWarning);
					d_currentScan.setErrorString(QString(""));
				}

				d_currentState = WaitingForRedo;
			}

			//addPointData returns true if point is now complete
			if(d_currentScan.addPointData(l))
			{
				if(d_currentState == WaitingForRedo)
					beginPoint();
				else
				{
					emit plotData(d_plotDataCache,x);
					emit pointComplete(d_currentScan.completedPoints());
					emit statusMessage(QString("Acquiring (%1/%2)").arg(d_currentScan.completedPoints())
								    .arg(d_currentScan.totalPoints()));
					checkScanComplete();
				}
			}
		}
	}
}

void AcquisitionManager::beginPoint()
{



    if(d_currentState != Paused )
    {
        if(d_currentState == Acquiring || d_currentState == WaitingForRedo)
        {
            d_plotDataCache.clear();
            d_currentState = WaitingForFrequency;
            if(d_currentScan.type() == Scan::LaserScan)
                emit startLaserPoint(d_currentScan.currentPos());
            else
                emit startCombPoint(d_currentScan.combShift());
        }

    }

}

void AcquisitionManager::frequencyReady()
{
    if(d_currentState == WaitingForFrequency)
    {
		d_currentState = WaitingForLockCheck;
        QTimer::singleShot(d_currentScan.delay(),[=](){ emit checkLock(d_currentScan.isHardwareActive(QString("cavityPZT")),d_currentScan.isHardwareActive(QString("frequencyCounter"))); });

	}

}

void AcquisitionManager::lockCheckComplete(bool locked, double cavityVoltage,double counterF)
{//add AOM manual relock call here

	if(d_currentState == WaitingForLockCheck)
	{
		if(d_currentScan.isAutoLockEnabled() && locked)
		{
            if(d_currentScan.isHardwareActive(QString("cavityPZT")))
            {
                auto range = d_currentScan.cavityPZTRange();
                if(cavityVoltage < range.first || cavityVoltage > range.second)
                    locked = false;
            }
		}
    }

		if(locked)
		{
            if(d_currentScan.isHardwareActive(QString("frequencyCounter")))
            {
                if(counterF > d_currentScan.counterRange().second||counterF<d_currentScan.counterRange().first)
                {
                    emit pumpRelock(counterF > d_currentScan.counterRange().second);//check if correct signal

                }
                else
                {
                    d_currentState = Acquiring;
                    emit getPointData();
                }
            }
            else
            {
                d_currentState = Acquiring;
                emit getPointData();
            }
		}
		else
        {
			if(d_currentScan.isAutoLockEnabled())
			{
                if(d_currentScan.isHardwareActive(QString("frequencyCounter")))
                {
                    if(counterF > d_currentScan.counterRange().second||counterF<d_currentScan.counterRange().first)
                    {
                        d_currentState = WaitingForAutoLock;
                        emit pumpRelock(counterF > d_currentScan.counterRange().second);

                    }
                    else
                    {
                        d_currentState = WaitingForAutoLock;
                        emit requestAutoLock();
                    }


                }
                else
                {
                    d_currentState = WaitingForAutoLock;

                    emit requestAutoLock();
                }
			}
            else
			{
				if(d_currentScan.isAbortOnUnlock())
                {
                    d_currentScan.setErrorString(QString("Aborted because lock was lost."));
					abortScan();
                }
				else
                {//add fcounter rails here
                    if(d_currentScan.isHardwareActive(QString("frequencyCounter")))
                    {
                        if(counterF > d_currentScan.counterRange().second||counterF<d_currentScan.counterRange().first)
                        {
                            d_currentState = WaitingForManualLock;
                            emit pumpRelock(counterF > d_currentScan.counterRange().second);
                        }
                        else
                        {
                            d_currentState = WaitingForManualLock;
                            emit requestManualLock();
                        }
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

void AcquisitionManager::manualPumpRelockComplete(bool abort)
{
    if(abort)
        abortScan();
    else
    {
        d_currentState = WaitingForFrequency;
        frequencyReady();
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

void AcquisitionManager::lockStateUpdate(bool locked)
{
    if(d_currentState == Acquiring && !locked && d_currentScan.isHardwareActive(QString("cavityPZT")))
	{
		emit statusMessage(QString("Lost lock, redoing point after relock."));
		d_currentState = WaitingForRedo;
		d_currentScan.setPointRedo();
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
	d_currentScan.finalSave();
	emit scanComplete(d_currentScan);
	d_currentState = Idle;
}

void AcquisitionManager::pauseScan() //added to pause/resume in the middle of scans. CRM
{
    if(d_currentState != Paused)
    {
        d_currentState = Paused;
    }
    else
    {
        d_currentState = Acquiring;
        beginPoint();
    }

}

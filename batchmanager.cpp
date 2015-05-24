#include "batchmanager.h"

BatchManager::BatchManager(BatchManager::BatchType b)
    : QObject(), d_type(b)
{

}

BatchManager::~BatchManager()
{

}

void BatchManager::scanComplete(const Scan s)
{
    if(!s.isInitialized())
    {
        writeReport();
        emit batchComplete(true);
        return;
    }

    emit logMessage(s.endLogMessage(),s.endLogCode());

    processScan(s);
    if(!s.isAborted() && !isComplete())
	   beginNextScan();
    else
    {
        writeReport();
	   emit batchComplete(s.isAborted());
    }

}

void BatchManager::beginNextScan()
{
    emit beginScan(nextScan());
}


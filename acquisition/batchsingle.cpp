#include "batchsingle.h"

BatchSingle::BatchSingle(Scan s) : BatchManager(BatchManager::SingleScan), d_scan(s)
{

}

BatchSingle::~BatchSingle()
{

}



void BatchSingle::writeReport()
{
}

void BatchSingle::processScan(const Scan s)
{
	Q_UNUSED(s)
}

Scan BatchSingle::nextScan()
{
	return d_scan;
}

bool BatchSingle::isComplete()
{
	return true;
}

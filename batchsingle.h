#ifndef BATCHSINGLE_H
#define BATCHSINGLE_H

#include "batchmanager.h"

class BatchSingle : public BatchManager
{
	Q_OBJECT
public:
	BatchSingle(Scan s);
	~BatchSingle();

	// BatchManager interface
protected:
	void writeReport();
	void processScan(const Scan s);
	Scan nextScan();
	bool isComplete();

private:
	Scan d_scan;
};

#endif // BATCHSINGLE_H

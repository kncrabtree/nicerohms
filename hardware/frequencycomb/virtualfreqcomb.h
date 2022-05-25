#ifndef VIRTUALFREQCOMB_H
#define VIRTUALFREQCOMB_H

#include "frequencycomb.h"

class VirtualFreqComb : public FrequencyComb
{
	Q_OBJECT
public:
	VirtualFreqComb(QObject *parent = nullptr);
	~VirtualFreqComb();

	// HardwareObject interface
public slots:
	bool testConnection();
	void initialize();
	Scan prepareForScan(Scan scan);
	void beginAcquisition();
	void endAcquisition();
	void readPointData();

	// FrequencyComb interface
public slots:
	FreqCombData readComb();
	bool setRepRate(double target);

};

#endif // VIRTUALFREQCOMB_H

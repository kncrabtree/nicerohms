#ifndef FREQUENCYCOMB_H
#define FREQUENCYCOMB_H

#include "hardwareobject.h"

#include "freqcombdata.h"

class FrequencyComb : public HardwareObject
{
	Q_OBJECT
public:
	FrequencyComb(QObject *parent = nullptr);
    virtual ~FrequencyComb();

	double pumpSign();
	double signalSign();

	void setDeltaNOverride(int dN = -1);

signals:
	void combUpdate(const FreqCombData);
	void repRateUpdate(double);


public slots:
	virtual FreqCombData readComb() =0;
	virtual bool setRepRate(double target) =0;
	void setIdlerFreq(double f) { d_currentIdlerFreq = f; }
	void setAOMFreq(double f){ d_currentAOMFreq = f; }
	void setSignalBeat(bool pos){ d_signalBeatPositive = pos; }
	void setPumpBeat(bool pos){ d_pumpBeatPositive = pos; }
	FreqCombData getLastMeasurement() const { return d_lastMeasurement; }

protected:
	bool d_pumpBeatPositive, d_signalBeatPositive, d_overrideNext;
	double d_currentDDSFreq, d_currentIdlerFreq, d_currentAOMFreq;

	int d_nextDeltaN;

	FreqCombData d_lastMeasurement;

};

#if NICEROHMS_FREQCOMB == 1
#include "menlocomb.h"
class MenloComb;
typedef MenloComb FreqCombHardware;
#else
#include "virtualfreqcomb.h"
class VirtualFreqComb;
typedef VirtualFreqComb FreqCombHardware;
#endif

#endif // FREQUENCYCOMB_H

#include "frequencycomb.h"

FrequencyComb::FrequencyComb(QObject *parent) :
	HardwareObject(parent), d_pumpBeatPositive(true), d_signalBeatPositive(true), d_overrideNext(false), d_currentDDSFreq(0.0),
    d_currentIdlerFreq(0.0), d_currentAOMFreq(0.0),d_currentCounterFreq(0.0), d_nextDeltaN(-1)
{
	d_key = QString("freqComb");
}

FrequencyComb::~FrequencyComb()
{

}

double FrequencyComb::pumpSign()
{
	return d_pumpBeatPositive ? 1.0 : -1.0;
}

double FrequencyComb::signalSign()
{
	return d_signalBeatPositive ? 1.0 : -1.0;
}

void FrequencyComb::setDeltaNOverride(int dN)
{
	if(dN > 0)
	{
        d_overrideNext = true; //originally true
		d_nextDeltaN = dN;
	}
	else
    {
        d_overrideNext = false;//originally false

    }

}




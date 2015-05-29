#include "wavemeterreadcontroller.h"

WavemeterReadController::WavemeterReadController(int numReads, Wavemeter::WavemeterState finalState, QObject *parent) :
    QObject(parent), d_targetReads(numReads), d_signalReadsComplete(0), d_pumpReadsComplete(0), d_currentWmState(Wavemeter::Unknown), d_finalWmState(finalState), d_currentControlState(Reading),
    d_signalMean(0.0), d_signalSumSq(0.0), d_pumpMean(0.0), d_pumpSumSq(0.0), d_aborted(false)
{
}

WavemeterReadController::~WavemeterReadController()
{

}

double WavemeterReadController::signalStDev() const
{
    if(d_signalReadsComplete < 2)
        return -1.0;

    return sqrt(d_signalSumSq/static_cast<double>(d_signalReadsComplete - 1));
}

double WavemeterReadController::pumpStDev() const
{
    if(d_pumpReadsComplete < 2)
        return -1.0;

    return sqrt(d_pumpSumSq/static_cast<double>(d_pumpReadsComplete - 1));
}

void WavemeterReadController::signalReadComplete(double freq)
{
    //compute new mean and sum sq of deviations
    //note: this is numerically stable!

    //only process if we're in the correct state
    if(d_signalReadsComplete < d_targetReads && d_currentControlState != Done)
    {
        d_signalReadsComplete++;
        double delta = freq - d_signalMean;
        d_signalMean += delta/static_cast<double>(d_signalReadsComplete);
        d_signalSumSq += delta*(freq - d_signalMean);

        //now, if we've reached the limit, we may need to read the other value, request a final flip, or we're done
        if(d_signalReadsComplete >= d_targetReads)
        {
            if(d_pumpReadsComplete < d_targetReads) //need to read pump
                emit flipRequest();
            else if(d_finalWmState == Wavemeter::Signal)
                emit readsComplete();
            else
            {
                d_currentControlState = WaitingForFinalFlip;
                emit flipRequest();
            }
        }
    }
}

void WavemeterReadController::pumpReadComplete(double freq)
{
    //compute new mean and sum sq of deviations
    //note: this is numerically stable!

    //only process if we're in the correct state
    if(d_pumpReadsComplete < d_targetReads && d_currentControlState != Done)
    {
        d_pumpReadsComplete++;
        double delta = freq - d_pumpMean;
        d_pumpMean += delta/static_cast<double>(d_pumpReadsComplete);
        d_pumpSumSq += delta*(freq - d_pumpMean);

        //now, if we've reached the limit, we may need to read the other value, request a final flip, or we're done
        if(d_pumpReadsComplete >= d_targetReads)
        {
            if(d_signalReadsComplete < d_targetReads) //need to read signal
                emit flipRequest();
            else if(d_finalWmState == Wavemeter::Pump)
                emit readsComplete();
            else
            {
                d_currentControlState = WaitingForFinalFlip;
                emit flipRequest();
            }
        }
    }
}

void WavemeterReadController::flipComplete()
{
    if(d_currentControlState == WaitingForFinalFlip)
        emit readsComplete(d_aborted);
}

void WavemeterReadController::abort()
{
    d_aborted = true;
    d_currentControlState = Done;
    emit readsComplete(true);
}


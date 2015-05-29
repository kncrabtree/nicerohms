#include "wavemeterreadcontroller.h"

WavemeterReadController::WavemeterReadController(int numReads, QObject *parent) :
    QObject(parent), d_targetReads(numReads), d_readsComplete(0), d_freqMean(0.0), d_freqSumSq(0.0), d_aborted(false)
{
}

WavemeterReadController::~WavemeterReadController()
{

}

double WavemeterReadController::freqStDev() const
{
    if(d_readsComplete < 2)
        return -1.0;

    return sqrt(d_freqSumSq/static_cast<double>(d_readsComplete - 1));
}

void WavemeterReadController::readComplete(double freq)
{
    //compute new mean and sum sq of deviations
    //note: this is numerically stable!

    //only process if we're in the correct state
    if(d_readsComplete < d_targetReads && !d_aborted)
    {
        d_readsComplete++;
        double delta = freq - d_freqMean;
        d_freqMean += delta/static_cast<double>(d_readsComplete);
        d_freqSumSq += delta*(freq - d_freqMean);

        //if we've reached the limit, we're done
        if(d_readsComplete >= d_targetReads)
            emit readsComplete();
    }
}

void WavemeterReadController::abort()
{
    d_aborted = true;
    emit readsComplete(true);
}


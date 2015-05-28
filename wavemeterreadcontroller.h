#ifndef WAVEMETERREADCONTROLLER_H
#define WAVEMETERREADCONTROLLER_H

#include <QObject>

#include "wavemeter.h"

class WavemeterReadController : public QObject
{
    Q_OBJECT
public:
    explicit WavemeterReadController(int numReads, Wavemeter::WavemeterState finalState, QObject *parent = 0);
    ~WavemeterReadController();

    enum ControlState {
        Reading,
        WaitingForFinalFlip
    };

    bool wasAborted() const { return d_aborted; }
    double signalMean() const { return d_signalMean; }
    double pumpMean() const { return d_pumpMean; }
    double signalStDev() const;
    double pumpStDev() const;

signals:
    void readsComplete(bool aborted = false);
    void flipRequest();

public slots:
    void signalReadComplete(double freq);
    void pumpReadComplete(double freq);
    void flipComplete();

private:
    int d_targetReads;
    int d_signalReadsComplete, d_pumpReadsComplete;
    Wavemeter::WavemeterState d_currentWmState, d_finalWmState;
    ControlState d_currentControlState;

    double d_signalMean, d_signalSumSq, d_pumpMean, d_pumpSumSq;
    bool d_aborted;



};

#endif // WAVEMETERREADCONTROLLER_H

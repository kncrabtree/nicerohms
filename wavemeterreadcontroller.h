#ifndef WAVEMETERREADCONTROLLER_H
#define WAVEMETERREADCONTROLLER_H

#include <QObject>

#include "wavemeter.h"

class WavemeterReadController : public QObject
{
    Q_OBJECT
public:
    explicit WavemeterReadController(int numReads, QObject *parent = 0);
    ~WavemeterReadController();

    bool wasAborted() const { return d_aborted; }
    double freqMean() const { return d_freqMean; }
    double freqStDev() const;

signals:
    void readsComplete(bool aborted = false);

public slots:
    void readComplete(double freq);
    void abort();

private:
    int d_targetReads;
    int d_readsComplete;

    double d_freqMean, d_freqSumSq;
    bool d_aborted;



};

#endif // WAVEMETERREADCONTROLLER_H

#ifndef MENLOCOMB_H
#define MENLOCOMB_H
#include "frequencycomb.h"
#include <QNetworkRequest>

class MenloComb : public FrequencyComb
{
    Q_OBJECT
public:
    MenloComb(QObject *parent = nullptr);
    ~MenloComb();
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

#endif // MENLOCOMB_H

#ifndef AOM8657A_H
#define AOM8657A_H

#include "aomsynthesizer.h"

class GpibController;

class Aom8657A : public AomSynthesizer
{
    Q_OBJECT
public:
    Aom8657A(GpibController *c, QObject *parent = nullptr);
    ~Aom8657A();
    // HardwareObject interface
public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void beginAcquisition();
    void endAcquisition();
    void readPointData();

    // AomSynthesizer interface
protected:
    void setFrequency(double f);
    double readFrequency();
};

#endif // AOM8657A_H

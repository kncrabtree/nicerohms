#ifndef CAVITYMDT694B_H
#define CAVITYMDT694B_H
#include "cavitypztdriver.h"

class CavityMDT694B : public CavityPZTDriver
{
public:
    CavityMDT694B(QObject *parent = nullptr);
    ~CavityMDT694B();

//Hardware implementation
public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void beginAcquisition();
    void endAcquisition();
    void readPointData();

    // CavityPZTDriver interface
public slots:
    double readVoltage();
};

#endif // CAVITYMDT694B_H

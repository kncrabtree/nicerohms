#ifndef PROLOGIXUSB_H
#define PROLOGIXUSB_H

#include "gpibcontroller.h"

class PrologixUSB : public GpibController
{
    Q_OBJECT
public:
    PrologixUSB(QObject *parent = nullptr);
    ~PrologixUSB();

    // HardwareObject interface
public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void beginAcquisition();
    void endAcquisition();
    void readPointData();

    // GpibController interface
protected:
    void readAddress();
    void setAddress(int a);
};

#endif // PROLOGIXUSB_H

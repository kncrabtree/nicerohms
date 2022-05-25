#ifndef BRISTOL621_H
#define BRISTOL621_H

#include "wavemeter.h"

class Bristol621 : public Wavemeter
{
    Q_OBJECT
public:
    Bristol621(QObject *parent = nullptr);
    ~Bristol621();

    // HardwareObject interface
public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void readPointData();

    // Wavemeter interface
protected:
    double read();

};

#endif // BRISTOL621_H

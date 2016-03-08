#ifndef LaserMDT694B_H
#define LaserMDT694B_H
#include <QSerialPortInfo>
#include "laser.h"

class LaserMDT694B : public Laser
{
    Q_OBJECT
public:
    LaserMDT694B(QObject *parent = nullptr);
    ~LaserMDT694B();

    // HardwareObject interface
public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void beginAcquisition();
    void endAcquisition();
    void readPointData();

    //Laser interface
public slots:
    double readPosition();
    double estimateFrequency();


protected:
    double setPosition(double target);
    void disableEcho();
    void calibrate();
    double offset;
    double parse(QByteArray response);

};

#endif // LaserMDT694B_H

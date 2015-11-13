#ifndef IOBOARDU6_H
#define IOBOARDU6_H
#include "ioboard.h"
#include "labjackusb.h"
#include "u6.h"
#include <QObject>
#include <QDebug>
class IOBoardU6 : public IOBoard
{
    Q_OBJECT
public:
    IOBoardU6(QObject *parent = nullptr);
    ~IOBoardU6();

signals:

    // HardwareObject interface
public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void beginAcquisition();
    void endAcquisition();
    void readPointData();

    // IOBoard interface
public slots:
    bool readCavityLocked();

protected:
    void setCavityPZTVoltage(double v);
    void setCavityLockOverride(bool unlock);

private:
    QList<QPair<bool,NicerOhms::LabJackRange>> d_analogConfig;
    QList<QPair<int,bool>> d_digitalConfig;
    HANDLE u6Handle;
    u6CalibrationInfo calInfo;
};

#endif // IOBOARDU6_H

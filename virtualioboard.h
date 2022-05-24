#ifndef VIRTUALIOBOARD_H
#define VIRTUALIOBOARD_H

#include "ioboard.h"

class VirtualIOBoard : public IOBoard
{
	Q_OBJECT
public:
	VirtualIOBoard(QObject *parent = nullptr);
	~VirtualIOBoard();

	// HardwareObject interface
public slots:
    bool testConnection() override;
    void initialize() override;
    Scan prepareForScan(Scan scan) override;
    void beginAcquisition() override;
    void endAcquisition() override;
    void readPointData() override;
    void relock() override;

	// IOBoard interface
public slots:
    bool readCavityLocked() override;

protected:
    void setCavityPZTVoltage(double v) override;
    void setCavityLockOverride(bool unlock) override;

private:
	QList<QPair<bool,NicerOhms::LabJackRange>> d_analogConfig;
	QList<QPair<int,bool>> d_digitalConfig;

    // IOBoard interface
public slots:
    void relockPump() override;
    void holdIntegrator(bool hold) override;
};

#endif // VIRTUALIOBOARD_H

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
};

#endif // VIRTUALIOBOARD_H

#ifndef VIRTUALWAVEMETER_H
#define VIRTUALWAVEMETER_H

#include "wavemeter.h"

class VirtualWavemeter : public Wavemeter
{
	Q_OBJECT
public:
	VirtualWavemeter(QObject *parent = nullptr);
	~VirtualWavemeter();

	// HardwareObject interface
public slots:
	bool testConnection();
	void initialize();
	Scan prepareForScan(Scan scan);
	void readPointData();

	// Wavemeter interface
protected:
	double read();

    // Wavemeter interface
public slots:
    void flipComplete();
};

#endif // VIRTUALWAVEMETER_H

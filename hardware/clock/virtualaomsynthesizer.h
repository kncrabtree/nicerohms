#ifndef VIRTUALAOMSYNTHESIZER_H
#define VIRTUALAOMSYNTHESIZER_H

#include "aomsynthesizer.h"

class GpibController;

class VirtualAomSynthesizer : public AomSynthesizer
{
	Q_OBJECT
public:
	VirtualAomSynthesizer(GpibController *c, QObject *parent = nullptr);
	~VirtualAomSynthesizer();

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

#endif // VIRTUALAOMSYNTHESIZER_H

#ifndef AOMSYNTHESIZER_H
#define AOMSYNTHESIZER_H

#include "hardwareobject.h"

class AomSynthesizer : public HardwareObject
{
	Q_OBJECT
public:
	AomSynthesizer(QObject *parent = nullptr);
	~AomSynthesizer();

signals:
	void frequencyUpdate(double);

protected:
	virtual void setFrequency(double f) =0;
	virtual double readFrequency() =0;

	double d_frequency;
};

#if NICEROHMS_AOMSYNTHESIZER == 1
#include "xxxxx.h"
class XXXXX;
typedef XXXXX AomSynthesizerHardware;
#else
#include "virtualaomsynthesizer.h"
class VirtualAomSynthesizer;
typedef VirtualAomSynthesizer AomSynthesizerHardware;
#endif

#endif // AOMSYNTHESIZER_H

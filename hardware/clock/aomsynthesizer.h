#ifndef AOMSYNTHESIZER_H
#define AOMSYNTHESIZER_H

#include <hardware/core/hardwareobject.h>

class AomSynthesizer : public HardwareObject
{
	Q_OBJECT
public:
	AomSynthesizer(QObject *parent = nullptr);
    virtual ~AomSynthesizer();

signals:
	void frequencyUpdate(double);

public slots:
	double getLastFrequency() { return d_frequency; }
	virtual void setFrequency(double f) =0;
	virtual double readFrequency() =0;

    double lowTrip() const { return d_lowTrip; }
    double highTrip() const { return d_highTrip; }
    void readTripParameters();

protected:
    double d_frequency, d_lowTrip, d_highTrip;
};

#if NICEROHMS_AOMSYNTH == 1
#include "aom8657a.h"
class Aom8657A;
typedef Aom8657A AomSynthesizerHardware;
#else
#include "virtualaomsynthesizer.h"
class VirtualAomSynthesizer;
typedef VirtualAomSynthesizer AomSynthesizerHardware;
#endif

#endif // AOMSYNTHESIZER_H

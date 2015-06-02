#include "aomsynthesizer.h"

AomSynthesizer::AomSynthesizer(QObject *parent) :
	HardwareObject(parent)
{
	d_key = QString("aomSynth");
	d_isCritical = false;
}

AomSynthesizer::~AomSynthesizer()
{

}

void AomSynthesizer::readTripParameters()
{
    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    d_lowTrip = s.value(QString("%1/%2/lowTrip").arg(d_key).arg(d_subKey),158e6).toDouble();
    d_highTrip = s.value(QString("%1/%2/highTrip").arg(d_key).arg(d_subKey),210e6).toDouble();
}


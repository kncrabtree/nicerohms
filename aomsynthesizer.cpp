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


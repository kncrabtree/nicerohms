#include "aomsynthesizer.h"

AomSynthesizer::AomSynthesizer(QObject *parent) :
	HardwareObject(parent)
{
	d_key = QString("aomSynth");
}

AomSynthesizer::~AomSynthesizer()
{

}


#include "laser.h"

#include <QSettings>
#include <QApplication>
#include <QTimer>

Laser::Laser(QObject *parent) :
	HardwareObject(parent), d_slewing(false), d_currentPos(0.0), d_slewStep(1.0), d_slewInterval(100)
{
	d_key = QString("laser");
}

Laser::~Laser()
{

}

void Laser::readSlewParameters()
{
	if(d_slewing)
		return;

	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    d_slewStep = s.value(QString("%1/%2/slewStep").arg(d_key).arg(d_subKey),1).toDouble();
	d_slewInterval = s.value(QString("%1/%2/slewInterval").arg(d_key).arg(d_subKey),100).toInt();

}

void Laser::slewToPosition(double target)
{
	if(d_slewing)
		return;


	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	double minPos = s.value(QString("%1/%2/minPos").arg(d_key).arg(d_subKey),0.0).toDouble();
	double maxPos = s.value(QString("%1/%2/maxPos").arg(d_key).arg(d_subKey),100.0).toDouble();

	double realTarget = qBound(minPos,target,maxPos);

	if(qAbs(d_currentPos - realTarget) > d_slewStep)
	{
		d_slewing = true;
		emit slewStarting();

		d_slewTarget = realTarget;
		nextSlewPoint();
	}
	else
	{
		setPosition(realTarget);
		emit slewComplete();
	}
}

void Laser::nextSlewPoint()
{
	if(!d_slewing)
	{
		emit slewComplete();
		return;
	}

	double direction = d_slewTarget > d_currentPos ? 1.0 : -1.0;

	double nextPos = d_currentPos + direction*d_slewStep;

	if(direction>0.0)
		nextPos = qMin(nextPos,d_slewTarget);
	else
		nextPos = qMax(nextPos,d_slewTarget);

	setPosition(nextPos);

	if(qFuzzyCompare(nextPos,d_slewTarget))
	{
		d_slewing = false;
		emit slewComplete();
	}
	else
		QTimer::singleShot(d_slewInterval,this,&Laser::nextSlewPoint);

}

void Laser::abortSlew()
{
	d_slewing = false;
}


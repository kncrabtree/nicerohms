#include "laser.h"

#include <memory>

#include <QSettings>
#include <QApplication>
#include <QTimer>

Laser::Laser(QObject *parent) :
	HardwareObject(parent), d_slewing(false), d_currentPos(0.0), d_slewStep(1.0), d_slewInterval(100)
{
	p_slewTimer = new QTimer(this);
}

Laser::~Laser()
{

}

void Laser::readSlewParameters()
{
	if(d_slewing)
		return;

	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	d_slewStep = s.value(QString("%1/%2/slewStep").arg(d_key).arg(d_subKey),1.0).toDouble();
	d_slewInterval = s.value(QString("%1/%2/slewInterval").arg(d_key).arg(d_subKey),100).toInt();

	p_slewTimer->setInterval(d_slewInterval);
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

		double direction = target > d_currentPos ? 1.0 : -1.0;
		double nextPos = d_currentPos + direction*d_slewStep;
		setPosition(nextPos);

		std::unique_ptr<QMetaObject::Connection> pconn{new QMetaObject::Connection};
		QMetaObject::Connection &conn = *pconn;
		conn = connect(p_slewTimer,&QTimer::timeout,this,[=](){
			if(qAbs(d_currentPos - realTarget) > d_slewStep)
			{
				double direction = realTarget > d_currentPos ? 1.0 : -1.0;
				double nextPos = d_currentPos + direction*d_slewStep;
				setPosition(nextPos);
			}
			else
			{
				d_slewing = false;
				p_slewTimer->stop();
				disconnect(conn);
				setPosition(realTarget);
				emit slewComplete();
			}
		});
		p_slewTimer->start();
	}
	else
	{
		setPosition(realTarget);
		emit slewComplete();
	}
}


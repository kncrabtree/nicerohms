#include "scan.h"

#include <QSettings>
#include <QApplication>

Scan::Scan() : data(new ScanData)
{

}

Scan::Scan(const Scan &rhs) : data(rhs.data)
{

}

Scan &Scan::operator=(const Scan &rhs)
{
	if (this != &rhs)
		data.operator=(rhs.data);
	return *this;
}

Scan::~Scan()
{

}

bool Scan::hardwareSuccess() const
{
	return data->hardwareSuccess;
}

void Scan::setHardwareFailed()
{
	data->hardwareSuccess = false;
}

void Scan::setInitialized()
{
	data->isInitialized = true;

	data->startTime = QDateTime::currentDateTime();

	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	int num = s.value(QString("exptNum"),1).toInt();
	data->number = num;

}


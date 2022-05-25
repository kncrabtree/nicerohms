#include "frequencycounter.h"

FrequencyCounter::FrequencyCounter(QObject *parent) :
    HardwareObject(parent), d_scanActive(false)
{
    d_key = QString("frequencyCounter");
    d_isCritical = false;

    p_timer = new QTimer(this);

    connect(p_timer,&QTimer::timeout,this,&FrequencyCounter::read);
    connect(this,&HardwareObject::hardwareFailure,p_timer,&QTimer::stop);


}

FrequencyCounter::~FrequencyCounter()
{

}

void FrequencyCounter::readTimerInterval()
{
    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    int timerInterval = s.value(QString("%1/%2/readInterval").arg(d_key).arg(d_subKey),200).toInt();
    p_timer->setInterval(timerInterval);
}

void FrequencyCounter::beginAcquisition()
{
    p_timer->stop();
    d_scanActive = true;
}

void FrequencyCounter::endAcquisition()
{
    p_timer->start();
    d_scanActive = false;

}

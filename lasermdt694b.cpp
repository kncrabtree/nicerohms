#include "lasermdt694b.h"
#include "rs232instrument.h"
#include <QDebug>
LaserMDT694B::LaserMDT694B(QObject *parent)
    : Laser(parent)
{
    d_subKey = QString("MDT694b");
    d_prettyName = QString("Laser");
    d_isCritical = true;

    p_comm = new Rs232Instrument(d_key,d_subKey,this);
    offset = 0;
    connect(p_comm,&CommunicationProtocol::logMessage,this,&LaserMDT694B::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){emit hardwareFailure();});



    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    s.setValue(QString("%1/%2/controlStep").arg(d_key).arg(d_subKey),1.0);
    s.setValue(QString("%1/%2/decimals").arg(d_key).arg(d_subKey),4);
    s.setValue(QString("%1/%2/units").arg(d_key).arg(d_subKey),QString("V"));
    s.setValue(QString("%1/%2/minPos").arg(d_key).arg(d_subKey),0.0);
    s.setValue(QString("%1/%2/maxPos").arg(d_key).arg(d_subKey),150.0);


    s.sync();


}

LaserMDT694B::~LaserMDT694B()
{


}

bool LaserMDT694B::testConnection()
{

    p_comm->testConnection();
    readPosition();
    QByteArray response = "]\r";


    if(p_comm->queryCmd("%\r").right(2)==response)
    {
        emit connected();
        calibrate();
        return true;
    }

        emit hardwareFailure();
       emit logMessage(QString("Connection failed"),NicerOhms::LogError);
        connected(false,QString());

        return false;



}

void LaserMDT694B::initialize()
{
    QByteArray termChar = "]\r";
    p_comm->setReadOptions(500, true, termChar);
    testConnection();

}

Scan LaserMDT694B::prepareForScan(Scan scan)
{
    readSlewParameters();
    if(d_isActive)
        scan.addNumDataPoints(1);

    return scan;
}

void LaserMDT694B::beginAcquisition()
{

}

void LaserMDT694B::endAcquisition()
{

}

void LaserMDT694B::readPointData()
{
    if(d_isActive)
    {
        QList<QPair<QString,QVariant>> out;
        out.append(qMakePair(QString("laser"),readPosition()));//source of rounding?
        emit pointDataReadNoPlot(out);
    }
}

double LaserMDT694B::readPosition()
{
    d_currentPos = parse(p_comm->queryCmd("XR?\r"));


    emit laserPosChanged(d_currentPos);
    return d_currentPos;

}

double LaserMDT694B::estimateFrequency()
{
    readPosition();

    return (-2.115e-6*d_currentPos*d_currentPos*d_currentPos - 0.000185*d_currentPos*d_currentPos + 0.6606*d_currentPos + 2.818e5)*1e9;
}

double LaserMDT694B::setPosition(double target)
{

    p_comm->writeCmd(QString("XV%1\r").arg(target + offset).toLatin1());

    readPosition();

}

void LaserMDT694B::calibrate()
{
    //removes offset between read/write
    if(d_currentPos>=3)
    {
        double initial = d_currentPos;
        p_comm->writeCmd(QString("XV%1\r").arg(d_currentPos).toLatin1());
        offset = initial - parse(p_comm->queryCmd("XR?\r"));

    }
    else
    {
        p_comm->writeCmd(QString("XV3\r").toLatin1());
        double initial = d_currentPos;
        p_comm->writeCmd(QString("XV%1\r").arg(d_currentPos).toLatin1());
        offset = initial - parse(p_comm->queryCmd("XR?\r"));
    }


}

double LaserMDT694B::parse(QByteArray response)
{
    return response.right(7).left(5).toDouble();
}

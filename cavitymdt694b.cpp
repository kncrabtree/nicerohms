#include "cavitymdt694b.h"
#include "rs232instrument.h"

CavityMDT694B::CavityMDT694B(QObject *parent) :
    CavityPZTDriver(parent)
{
    d_subKey = QString("MDT694b");
    d_prettyName = QString("Cavity PZT");
    d_isCritical = false;

    p_comm = new Rs232Instrument(d_key,d_subKey,this);

    connect(p_comm,&CommunicationProtocol::logMessage,this,&CavityMDT694B::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){emit hardwareFailure();});

}

CavityMDT694B::~CavityMDT694B()
{

}

bool CavityMDT694B::testConnection()
{
    p_comm->testConnection();
    QByteArray response = "]\r";


    if(p_comm->queryCmd("%\r").right(2)==response)
    {
        emit connected(true,QString());

        return true;
    }

        emit hardwareFailure();
       emit logMessage(QString("Connection failed"),NicerOhms::LogError);
        connected(false,QString());

        return false;
}

void CavityMDT694B::initialize()
{
    QByteArray termChar = "]\r";
    p_comm->setReadOptions(500, true, termChar);
    testConnection();
}

Scan CavityMDT694B::prepareForScan(Scan scan)
{
    if(d_isActive)
        scan.addNumDataPoints(1);

    return scan;
}

void CavityMDT694B::beginAcquisition()
{
}

void CavityMDT694B::endAcquisition()
{
}

void CavityMDT694B::readPointData()
{
    if(d_isActive)
    {
        QList<QPair<QString,QVariant>> out;
        out.append(qMakePair(QString("CavityPZT"),readVoltage()));
        emit pointDataRead(out);
    }
}

double CavityMDT694B::readVoltage()
{
    double voltage = p_comm->queryCmd("XR?\r").right(7).left(5).toDouble();
//    double voltage = 50.0 + static_cast<double>((qrand()%2000)-1000)/1e3;
    emit cavityPZTUpdate(voltage);
    return voltage;
}

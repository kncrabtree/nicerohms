#include "bristol621.h"
#include <hardware/core/communication/tcpinstrument.h>

Bristol621::Bristol621(QObject *parent) :
    Wavemeter(parent)
{
    d_subKey = QString("Bristol621");
    d_prettyName = QString("Bristol Wavemeter");
    d_isCritical = false;

    p_comm = new TcpInstrument(d_key,d_subKey,this);
    connect(p_comm,&CommunicationProtocol::logMessage,this,&Bristol621::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

}

Bristol621::~Bristol621()
{

}

bool Bristol621::testConnection()
{
    p_timer->stop();
    p_comm->initialize();
    p_comm->testConnection();

    if(p_comm->queryCmd(QString('h').toLatin1()) != QByteArray())
    {
        readTimerInterval();

        p_timer->start();
        emit connected();
        return true;
    }

    emit connected(false,QString("could not connect"));


    readTimerInterval();

    p_timer->start();
    emit connected(false);
    return true;
}

void Bristol621::initialize()
{
    testConnection();

}

Scan Bristol621::prepareForScan(Scan scan)
{
    if(d_isActive)
        scan.addNumDataPoints(1);
    return scan;
}

void Bristol621::readPointData()
{
    if(d_isActive)
    {
        QList<QPair<QString,QVariant>> out;
        out.append(qMakePair(QString("wavemeter"),read()));
        emit pointDataRead(out);
    }
}

double Bristol621::read()
{
    d_currentFreq = p_comm->queryCmd(QString('h').toLatin1()).toDouble();
    if(d_currentFreq > 1e16)
        d_currentFreq = -1;
    emit freqUpdate(d_currentFreq);
    return d_currentFreq;
}

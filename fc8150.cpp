#include "fc8150.h"
#include "tcpinstrument.h"
#include <QDebug>
FC8150::FC8150(QObject *parent) :
    FrequencyCounter(parent)
{
    d_subKey = QString("FC8150");
    d_prettyName = QString("Uni Frequency Counter");
    d_isCritical = false;

    p_comm = new TcpInstrument(d_key,d_subKey,this);
    connect(p_comm,&CommunicationProtocol::logMessage,this,&FC8150::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

}

FC8150::~FC8150()
{

}

bool FC8150::testConnection()
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

void FC8150::initialize()
{
    testConnection();
}

Scan FC8150::prepareForScan(Scan scan)
{
    if(d_isActive)
        scan.addNumDataPoints(1);
    return scan;
}

void FC8150::readPointData()
{
    if(d_isActive)
    {
        QList<QPair<QString,QVariant>> out;
        out.append(qMakePair(QString("frequencyCounter"),read()));
        emit pointDataRead(out);
    }
}

double FC8150::read()
{
    d_currentFreq = p_comm->queryCmd(QString('h').toLatin1()).toDouble();
    if(d_currentFreq > 1e16)
        d_currentFreq = -1;//determine error trap

    emit freqUpdate(d_currentFreq);
    return d_currentFreq;
}


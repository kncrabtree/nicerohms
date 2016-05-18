#include "aom8657a.h"
#include "gpibinstrument.h"


Aom8657A::Aom8657A(GpibController *c, QObject *parent) :
    AomSynthesizer(parent)
{
    d_subKey = QString("8657A");
    d_prettyName = QString("AOM Synthesizer 8657A");
    d_isCritical = false;

    p_comm = new GpibInstrument(d_key,d_subKey,c,this);

    connect(p_comm,&CommunicationProtocol::logMessage,this,&Aom8657A::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

    d_frequency = 185e6;

}

Aom8657A::~Aom8657A()
{

}

bool Aom8657A::testConnection()
{
    bool success = p_comm->testConnection();

    //would normally send query command here
    //QByteArray resp = p_comm->queryCmd(QString("*IDN?\r\n"));
    //parse resp... check for success
    GpibInstrument *g = static_cast<GpibInstrument*>(p_comm);
    emit logMessage(QString("GPIB address: %1").arg(g->address()));

    if(success)
    {
        readTripParameters();
        setFrequency(d_frequency);

        emit connected();
        return true;
    }
    else
    {
        emit connected(false,QString("GPIB error."));
        return false;
    }
}

void Aom8657A::initialize()
{
    testConnection();
}

Scan Aom8657A::prepareForScan(Scan scan)
{
    if(d_isActive)
    {
        readTripParameters();
        scan.addNumDataPoints(1);
    }

    return scan;
}

void Aom8657A::beginAcquisition()
{

}
void Aom8657A::endAcquisition()
{

}

void Aom8657A::readPointData()
{
    if(d_isActive)
    {
        QList<QPair<QString,QVariant>> out;
        out.append(qMakePair(QString("aom"),readFrequency()));
        //consider whether AOM frequency needs to be plotted?
        emit pointDataRead(out);
    }
}

void Aom8657A::setFrequency(double f)
{

    d_frequency = f/1e6;

    p_comm->writeCmd(QString("FR%1MZ\r").arg(d_frequency).toLatin1());
    readFrequency();
}

double Aom8657A::readFrequency()
{
    //would normally do a query here
    //QByteArray resp = p_comm->queryCmd(QString("freq?\r\n"));
    // unfortunately the HP 8657a cannot respond -CRM
    emit frequencyUpdate(d_frequency);
    return d_frequency;
}

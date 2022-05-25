#include "prologixusb.h"
#include <hardware/core/communication/rs232instrument.h>
PrologixUSB::PrologixUSB(QObject *parent)  : GpibController(parent)
{
    d_subKey = QString("prologix");
    d_prettyName = QString("Prologix USB");

    d_isCritical = false;

    p_comm = new Rs232Instrument(d_key,d_subKey,this);

    connect(p_comm,&CommunicationProtocol::logMessage,this,&PrologixUSB::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });
}

PrologixUSB::~PrologixUSB()
{

}

bool PrologixUSB::testConnection()
{
    p_comm->testConnection();
    QByteArray response = "Prologix";

    if(p_comm->queryCmd("++ver\n").left(8) == response)
    {
        emit connected();
        return true;
    }

    emit connected(false,QString("Unable to connect"));
    return false;
}

void PrologixUSB::initialize()
{
    QByteArray termChar = "\n";
    p_comm->setReadOptions(100,true,termChar);
    testConnection();
}

Scan PrologixUSB::prepareForScan(Scan scan)
{
    return scan;

}

void PrologixUSB::beginAcquisition()
{

}
void PrologixUSB::endAcquisition()
{

}
void PrologixUSB::readAddress()
{
    d_currentAddress = p_comm->queryCmd(QString("++addr\n")).left(1).toInt();
}
void PrologixUSB::setAddress(int a)
{
    d_currentAddress = a;
    p_comm->writeCmd(QString("++addr %1\n").arg(a).toLatin1());

}

void PrologixUSB::readPointData()
{

}

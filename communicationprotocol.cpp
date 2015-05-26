#include "communicationprotocol.h"

CommunicationProtocol::CommunicationProtocol(CommType type, QString key, QString subKey, QObject *parent) :
    QObject(parent), d_type(type), d_key(QString("%1").arg(key)),
    d_useTermChar(false), d_timeOut(1000)
{
	Q_UNUSED(subKey)
}

CommunicationProtocol::~CommunicationProtocol()
{

}


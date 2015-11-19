#ifndef XMLINSTRUMENT_H
#define XMLINSTRUMENT_H
#include "communicationprotocol.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

class XmlInstrument : public CommunicationProtocol
{
    Q_OBJECT
public:
    XmlInstrument(QString key, QString subKey,QObject *parent = nullptr);
    ~XmlInstrument();

    // CommunicationProtocol interface
    QByteArray queryCmd(QString cmd);
    QIODevice *device() { return nullptr; }
    bool writeCmd(QString cmd);

public slots:
    void initialize();
    bool testConnection();
private:
    QNetworkAccessManager *d_xc;
};

#endif // XMLINSTRUMENT_H

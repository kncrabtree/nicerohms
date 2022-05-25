#include "xmlinstrument.h"
#include <QDebug>
XmlInstrument::XmlInstrument(QString key, QString subKey,QObject *parent) :
    CommunicationProtocol(CommunicationProtocol::Custom,key,subKey,parent)
{
}

XmlInstrument::~XmlInstrument()
{

}



QByteArray XmlInstrument::queryCmd(QString cmd)
{

    QByteArray auth = "Basic ";
    auth.append(QByteArray("comb:system").toBase64());


    QSettings s(QSettings::SystemScope, QApplication::organizationName(), QApplication::applicationName());

    s.beginGroup(d_key);
    QString ip= s.value(QString("uri").arg(d_key)).toString();



//    QUrl url(s.value(QString("URL"),QString("http://192.168.0.20:8123/RPC2")).toString());
    QUrl url(s.value(QString("URL"),QString("http://%1:8123/RPC2").arg(ip)).toString());
    QByteArray userAgent = s.value(QString("User-Agent"),QString("NICER-OHMS Acquisition Software")).toByteArray();

    QByteArray host = s.value(QString("Host"),QString("")).toByteArray();


    s.endGroup();


    QNetworkRequest nReq(url);
    nReq.setRawHeader("User-Agent",userAgent);
    nReq.setRawHeader("Host",host);
    nReq.setRawHeader("Authorization",auth);

    nReq.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));

    nReq.setHeader(QNetworkRequest::ContentLengthHeader,cmd.toLatin1().size());
    QEventLoop loop;
    QNetworkReply *nReply = d_xc->post(nReq,cmd.toLatin1());

    connect(d_xc,SIGNAL(finished(QNetworkReply*)),&loop,SLOT(quit()));
    loop.exec();

    QByteArray Reply = nReply->readAll();


    return Reply;
}

void XmlInstrument::initialize()
{
    d_xc = new QNetworkAccessManager(this);
}

bool XmlInstrument::testConnection()
{
    return true;
}

bool XmlInstrument::writeCmd(QString cmd)
{
    Q_UNUSED(cmd)
    return false;
}

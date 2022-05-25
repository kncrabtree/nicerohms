#include "menlocomb.h"
#include "frequencycomb.h"
#include <hardware/core/communication/xmlinstrument.h>
#include <QNetworkRequest>
#include <QNetworkAccessManager>


MenloComb::MenloComb(QObject *parent) :
    FrequencyComb(parent)
{
    d_subKey = QString("menlo");
    d_prettyName = QString("Menlo Frequency Comb");
    d_isCritical = false;

    p_comm = new XmlInstrument(d_key,d_subKey,this);
    connect(p_comm,&CommunicationProtocol::logMessage,this,&MenloComb::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

}

MenloComb::~MenloComb()
{

}

bool MenloComb::testConnection()
{

    QByteArray response = "<?xml";
    if(p_comm->queryCmd("hello").left(5)==response)
    {
        emit connected();
        return true;
    }
    else
    {
        emit connected(false,QString("Unable to connect"));
        return false;
    }

}

void MenloComb::initialize()
{

    testConnection();
}

Scan MenloComb::prepareForScan(Scan scan)
{
    if(d_isActive)
    {
        scan.addNumDataPoints(8);
    }

    return scan;
}

void MenloComb::beginAcquisition()
{

}
void MenloComb::endAcquisition()
{

}

void MenloComb::readPointData()
{
    if(d_isActive)
    {
        FreqCombData d = readComb();
        QList<QPair<QString,QVariant>> plotData;
        QList<QPair<QString,QVariant>> noPlotData;

        plotData.append(qMakePair(QString("reprate"),d.repRate()));
        plotData.append(qMakePair(QString("idler"),d.calculatedIdlerFreq()));
        noPlotData.append(qMakePair(QString("offsetBeat"),d.offsetBeat()));
        noPlotData.append(qMakePair(QString("pumpBeat"),d.pumpBeat()));
  //      qDebug() << "\tfinal pump " << d.pumpBeat() / 1e6;
        noPlotData.append(qMakePair(QString("signalBeat"),d.signalBeat()));
        noPlotData.append(qMakePair(QString("deltaN"),d.deltaN()));
        noPlotData.append(qMakePair(QString("replockv"),d.repRateLockVoltage()));
        noPlotData.append(qMakePair(QString("replocked"),d.repRateLocked()));

        emit pointDataRead(plotData);
        emit pointDataReadNoPlot(noPlotData);

    }
}

FreqCombData MenloComb::readComb()
{
    FreqCombData out;
    QDomDocument document;
    QByteArray response = p_comm->queryCmd("<?xml version=\"1.0\"?><methodCall><methodName>data.query</methodName><params><param><value><double>-1.0</double></value></param></params></methodCall>");

    document.setContent(response);

    out.parseXml(document);

    out.setBeatSigns(d_pumpBeatPositive,d_signalBeatPositive);
    //Supposed to use override, unless it is first point, then setDeltaN calculates first deltaN
    if(d_overrideNext)//changed from override next to >0
    {
        out.setDeltaN(d_nextDeltaN,d_currentAOMFreq,d_currentCounterFreq);//overload

        d_overrideNext = false;
    }
    else
    {
        d_nextDeltaN = out.setDeltaN(d_currentIdlerFreq,d_currentAOMFreq,d_currentCounterFreq);
    }

    d_lastMeasurement = out;
    d_currentDDSFreq = out.repRate()*10.0 - 980e6;

    emit combUpdate(out);
    return out;
}

bool MenloComb::setRepRate(double target)
{

    //normally, would check to make sure DDS frequency doesn't change too much
    //units are Hz!


    d_currentDDSFreq = (target*10.0)-(980e6);

    QString sDataQueryCommand = tr("<?xml version=\"1.0\"?><methodCall><methodName>comb.reprate.synth.setFreq</methodName><params><param><value><double>%1</double></value></param></params></methodCall>").arg(d_currentDDSFreq, 0, 'g', 10);
    p_comm->queryCmd(sDataQueryCommand);


    emit repRateUpdate(target);
    return true;

}


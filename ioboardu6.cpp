#include "ioboardu6.h"
#include "virtualinstrument.h"


IOBoardU6::IOBoardU6(QObject *parent) :
    IOBoard(parent)
{
    d_subKey = QString("u6");
    d_prettyName = QString("Labjack u6");
    d_isCritical = true;

    p_comm = new VirtualInstrument(d_key,this);
    connect(p_comm,&CommunicationProtocol::logMessage,this,&IOBoardU6::logMessage);
    connect(p_comm,&CommunicationProtocol::hardwareFailure,[=](){ emit hardwareFailure(); });

}

IOBoardU6::~IOBoardU6()
{
    closeUSBConnection(u6Handle);
}

bool IOBoardU6::testConnection()
{
    //test anaolog read to see if device is already connected
    double dummyVoltage;
    //eAIN(devHandle, &calInf, xChan, 15, &dblVoltage, 0, 0, 0, 0, 0, 0)
    if(eAIN(u6Handle,&calInfo,0,15,&dummyVoltage,0, 0, 0, 0, 0, 0)==0)
        closeUSBConnection(u6Handle);


    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    int u6LocalId = s.value(QString("%1/%2/serialNo").arg(d_key).arg(d_subKey),1).toInt();

    u6Handle = openUSBConnection(u6LocalId);

    getCalibrationInfo(u6Handle,&calInfo);
    if(u6Handle==NULL)
    {
        emit hardwareFailure();
        emit logMessage(QString("Unable to connect"),NicerOhms::LogError);
        emit connected(false,QString());
        return false;
    }
    else
    {

    p_lockReadTimer->stop();
    readIOSettings();
    readCavityLocked();

    p_lockReadTimer->start();
    emit connected(true,QString());
    return true;
    }
}

void IOBoardU6::initialize()
{

    testConnection();
}

Scan IOBoardU6::prepareForScan(Scan scan)
{
    //ioboard is always active
    readIOSettings();

    d_analogConfig = scan.ioboardAnalogConfig();
    d_digitalConfig = scan.ioboardDigitalConfig();

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    QString prefix = QString("IoBoard");

    int numPoints = 0;
    for(int i=0; i<d_analogConfig.size(); i++)
    {
        if(d_analogConfig.at(i).first)
        {
            numPoints++;
            QString name = s.value(QString("analogNames/ain%1").arg(i),QString("")).toString();
            QString r, units;
            switch(d_analogConfig.at(i).second)
            {
            case NicerOhms::LJR10V:
                r = QString("10");
                units = QString("V");
                break;
            case NicerOhms::LJR1V:
                r = QString("1");
                units = QString("V");
                break;
            case NicerOhms::LJR100mV:
                r = QString("100");
                units = QString("mV");
                break;
            case NicerOhms::LJR10mV:
                r = QString("10");
                units = QString("mV");
                break;
            }
            scan.addHeaderItem(prefix+QString("AIN%1Name").arg(i),name,QString(""));
            scan.addHeaderItem(prefix+QString("AIN%1Range").arg(i),r,QString(""));
        }
    }
    for(int i=0; i<d_digitalConfig.size(); i++)
    {
        if(d_digitalConfig.at(i).second)
        {
            numPoints++;
            int reserved = s.value(QString("%1/%2/reservedDigitalChannels").arg(d_key).arg(d_subKey),2).toInt();
            QString name = s.value(QString("digitalNames/din%1").arg(i+reserved)).toString();
            scan.addHeaderItem(prefix+QString("DIN%1Name").arg(i+reserved),name);
        }
    }
    scan.addNumDataPoints(numPoints);

    return scan;
}

void IOBoardU6::beginAcquisition()
{
}

void IOBoardU6::endAcquisition()
{
}

void IOBoardU6::readPointData()
{
    QList<QPair<QString,QVariant>> analog;
    QList<QPair<QString,QVariant>> digital;

    for(int i=0; i<d_analogConfig.size(); i++)
    {
        if(d_analogConfig.at(i).first)
        {
            double r;
            //eAIN(devHandle, &calInf, xChan, 15, &dblVoltage, 0, 0, 0, 0, 0, 0);



//            double r = static_cast<double>((qrand()%20000)-10000)/10000.0;
            switch (d_analogConfig.at(i).second) {
            case NicerOhms::LJR10V:
                analog.append(qMakePair(QString("ain%1").arg(i),r*10.0));
                eAIN(u6Handle,&calInfo,i,15,&r,LJ_rgBIP10V,0,0,0,0,0);
                break;
            case NicerOhms::LJR1V:
                analog.append(qMakePair(QString("ain%1").arg(i),r));
                eAIN(u6Handle,&calInfo,i,15,&r,LJ_rgBIP1V,0,0,0,0,0);
                break;
            case NicerOhms::LJR100mV:
                analog.append(qMakePair(QString("ain%1").arg(i),r*0.1));
                eAIN(u6Handle,&calInfo,i,15,&r,LJ_rgBIPP1V,0,0,0,0,0);
                break;
            case NicerOhms::LJR10mV:
                analog.append(qMakePair(QString("ain%1").arg(i),r*0.01));
                eAIN(u6Handle,&calInfo,i,15,&r,LJ_rgBIPP01V,0,0,0,0,0);
                break;
            default:
                break;
            }
        }
    }

    for(int i=0; i<d_digitalConfig.size(); i++)
    {
        if(d_digitalConfig.at(i).second)
        {
            long r;
            eDI(u6Handle,i,&r);
            bool b = r==1;

            digital.append(qMakePair(QString("din%1").arg(d_digitalConfig.at(i).first),b));
        }
    }

    emit pointDataRead(analog);
    emit pointDataReadNoPlot(digital);
}

bool IOBoardU6::readCavityLocked()
{
    long r;
    eDI(u6Handle,0,&r);
    bool on = r==1;

//    bool on = qrand() % 100;
    emit lockState(on);
    return on;
}

void IOBoardU6::setCavityPZTVoltage(double v)
{
    d_lastCavityVoltage = v;
}

void IOBoardU6::setCavityLockOverride(bool unlock)
{
    Q_UNUSED(unlock)
}

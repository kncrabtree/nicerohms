#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>

#include <QList>
#include <QThread>
#include <QPair>
#include <QVariant>


#include "datastructs.h"
#include "scan.h"
#include "freqcombdata.h"

class HardwareObject;
class Laser;
class LockIn;
class Wavemeter;
class CavityPZTDriver;
class IOBoard;
class GpibController;
class AomSynthesizer;
class FrequencyComb;

class HardwareManager : public QObject
{
    Q_OBJECT
public:
    explicit HardwareManager(QObject *parent = 0);
    ~HardwareManager();

signals:
    void logMessage(const QString, const NicerOhms::LogMessageCode = NicerOhms::LogNormal);
    void statusMessage(const QString);
    void allHardwareConnected(bool);
    /*!
     * \brief Emitted when a connection is being tested from the communication dialog
     * \param QString The HardwareObject key
     * \param bool Whether connection was successful
     * \param QString Error message
     */
    void testComplete(QString,bool,QString);
    void beginAcquisition();
    void abortAcquisition();
    void scanInitialized(Scan);
    void endAcquisition();
    void pointData(const QList<QPair<QString,QVariant>>, bool plot);
    void readPointData();

    void slewLaser(double);
    void laserPosUpdate(double);
    void laserSlewStarted();
    void laserSlewComplete();
    void abortSlew();

    void wavemeterFreqUpdate(double);
    void updateWavemeterTimer();
    void cavityPZTUpdate(double);

    void relockComplete(bool);
    void lockStateUpdate(bool);
    void lockStateCheck(bool locked, double cavityVoltage);
    void autoRelock();

    void aomSynthUpdate(double);

    void combUpdate(FreqCombData);
    void repRateUpdate(double);
    void setCombPumpBeat(bool);
    void setCombSignalBeat(bool);

public slots:
    void initialize();

    /*!
     * \brief Records whether hardware connection was successful
     * \param obj A HardwareObject that was tested
     * \param success Whether communication was sucessful
     * \param msg Error message
     */
    void connectionResult(HardwareObject *obj, bool success, QString msg);

    /*!
     * \brief Sets hardware status in d_status to false, disables program
     * \param obj The object that failed.
     *
     * TODO: Consider generating an abort signal here
     */
    void hardwareFailure(HardwareObject *obj, bool abort);
    void beginScanInitialization(Scan s);
    void completeScanInitialization(Scan s, bool stageOneSuccess = true, QString errorMsg = QString(""));
    void testObjectConnection(const QString type, const QString key);
    void testAllConnections();
    void getPointData();


    void checkLock();
    double checkCavityVoltage();

    double getAomFrequency();

    void readComb();
    void setCombIdlerFreq(double f);

private:
    QHash<QString,bool> d_status;
    void checkStatus();

    Laser *p_laser;
    LockIn *p_lockIn1, *p_lockIn2;
    Wavemeter *p_wavemeter;
    CavityPZTDriver *p_cavityPZT;
    IOBoard *p_iob;
    GpibController *p_gpibController;
    AomSynthesizer *p_aomSynth;
    FrequencyComb *p_freqComb;

    QList<QPair<HardwareObject*,QThread*> > d_hardwareList;

};

#endif // HARDWAREMANAGER_H

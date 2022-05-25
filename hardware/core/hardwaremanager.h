#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>

#include <QList>
#include <QThread>
#include <QPair>
#include <QVariant>
#include <QTimer>

#include <data/datastructs.h>
#include <data/scan.h>
#include <data/freqcombdata.h>
#include <hardware/frequencycounter/frequencycounter.h>

class HardwareObject;
class Laser;
class LockIn;
class Wavemeter;
class FrequencyCounter;
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
    void pauseAcquisition();
    void scanInitialized(Scan);
    void endAcquisition();
    void pointData(const QList<QPair<QString,QVariant>>, bool plot);
    void readPointData();
    void uiAbort();

    void slewLaser(double);
    void laserPosUpdate(double);
    void laserSlewStarted();
    void laserSlewComplete();
    void abortSlew();

    void wavemeterFreqUpdate(double);
    void counterFreqUpdate(double);
    void updateWavemeterTimer();
    void updateCounterTimer();
    void cavityPZTUpdate(double);

    void relockComplete(bool);
    void lockStateUpdate(bool);
    void lockStateCheck(bool locked, double cavityVoltage, double counterF);
    void autoRelock();
    void relockPump();
    void requestPumpManualRelock();

    void aomSynthUpdate(double);

    void combUpdate(FreqCombData);
    void repRateUpdate(double);
    void setCombPumpBeat(bool);
    void setCombSignalBeat(bool);
    void manualPumpRelockComp(bool abort);
    void readyForPoint();
    void setNextDn(int nextDN);

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
    void beginCombPoint(double shiftMHz);
    void testObjectConnection(const QString type, const QString key);
    void testAllConnections();
    void getPointData();
    void cleanUpAfterScan();

    double estimateLaserFrequency();

    void checkLock(bool pztEnabled,bool pumpLockEnabled);
    double checkCavityVoltage();
    void manualPumpRelockCheck(bool abort, bool tripH);

    double getAomFrequency();
    void setAomFrequency(double f);
    double aomLowTrip();
    double aomHighTrip();

    void readComb();
    void setCombRepRate(double f);
    void setCombIdlerFreq(double f);
    void setCombOverrideDN(int dN);
    FreqCombData getLastCombReading();

    double getCounterFrequency();
    void relockPumpToAom();
    void setIntegrator(bool hold);

private:
    QHash<QString,bool> d_status;
    void checkStatus();
    void centerPump();

    QTimer *p_intTimer;
    int relockStep;
    double relockFrequency;
    bool useLoop;

    Laser *p_laser;
    LockIn *p_lockIn1, *p_lockIn2;
    Wavemeter *p_wavemeter;
    FrequencyCounter *p_counter;
    CavityPZTDriver *p_cavityPZT;
    IOBoard *p_iob;
    GpibController *p_gpibController;
    AomSynthesizer *p_aomSynth;
    FrequencyComb *p_freqComb;


    QList<QPair<HardwareObject*,QThread*> > d_hardwareList;

};

#endif // HARDWAREMANAGER_H

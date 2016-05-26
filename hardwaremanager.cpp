#include "hardwaremanager.h"

#include <QSettings>

#include "hardwareobject.h"
#include "laser.h"
#include "lockin.h"
#include "wavemeter.h"
#include "frequencycounter.h"
#include "cavitypztdriver.h"
#include "ioboard.h"
#include "gpibcontroller.h"
#include "aomsynthesizer.h"
#include "wavemeterreadcontroller.h"
#include "frequencycomb.h"


#include <QDebug>

HardwareManager::HardwareManager(QObject *parent) : QObject(parent)
{

}

HardwareManager::~HardwareManager()
{
    //note, the hardwareObjects are deleted when the threads exit
    while(!d_hardwareList.isEmpty())
    {
        QPair<HardwareObject*,QThread*> p = d_hardwareList.takeFirst();
        if(p.second != nullptr)
        {
		   if(p.second->isRunning())
		   {
			   p.second->quit();
			   p.second->wait();
		   }
        }
        else
            p.first->deleteLater();
    }
}

void HardwareManager::initialize()
{    
	//Laser does not need its own thread
    p_laser = new LaserHardware();
    connect(this,&HardwareManager::slewLaser,p_laser,&Laser::slewToPosition);
    connect(p_laser,&Laser::laserPosChanged,this,&HardwareManager::laserPosUpdate);
    connect(p_laser,&Laser::slewStarting,this,&HardwareManager::laserSlewStarted);
    connect(p_laser,&Laser::slewComplete,this,&HardwareManager::laserSlewComplete);
    connect(this,&HardwareManager::uiAbort,p_laser,&Laser::abortSlew);
    d_hardwareList.append(qMakePair(p_laser,nullptr));

	//Lock ins may need their own threads since comms might be slow
	p_lockIn1 = new LockIn1Hardware(1);
	p_lockIn2 = new LockIn2Hardware(2);
	d_hardwareList.append(qMakePair(p_lockIn1,new QThread(this)));
	d_hardwareList.append(qMakePair(p_lockIn2,new QThread(this)));

	//wavemeter unlikely to need its own thread unless reads are very slow
	//calls are designed generally to be asynchronous
	p_wavemeter = new WavemeterHardware();
    connect(p_wavemeter,&Wavemeter::freqUpdate,this,&HardwareManager::wavemeterFreqUpdate);
	connect(this,&HardwareManager::updateWavemeterTimer,p_wavemeter,&Wavemeter::readTimerInterval);
	d_hardwareList.append(qMakePair(p_wavemeter,nullptr));

    //Frequency counter
    p_counter = new CounterHardware();
    connect(p_counter,&FrequencyCounter::freqUpdate,this,&HardwareManager::counterFreqUpdate);
    connect(this,&HardwareManager::updateCounterTimer,p_counter,&FrequencyCounter::readTimerInterval);
    d_hardwareList.append(qMakePair(p_counter,nullptr));

	//cavity pzt driver probably does not need to be in its own thread
	p_cavityPZT = new CavityPZTHardware();
	connect(p_cavityPZT,&CavityPZTDriver::cavityPZTUpdate,this,&HardwareManager::cavityPZTUpdate);
	d_hardwareList.append(qMakePair(p_cavityPZT,nullptr));

	//ioboard probably does not need its own thread
    p_iob = new IOBoardHardware();
    connect(p_iob,&IOBoard::relockComplete,this,&HardwareManager::relockComplete);
    connect(p_iob,&IOBoard::lockState,this,&HardwareManager::lockStateUpdate);
    connect(this,&HardwareManager::autoRelock,p_iob,&IOBoard::relock);
    connect(this,&HardwareManager::relockPump,p_iob,&IOBoard::relockPump);
    d_hardwareList.append(qMakePair(p_iob,nullptr));

	//GPIB controller may need its own thread, but may not...
	//If GPIB controller has its own thread, then all GPIB instruments MUST be in that same thread
	p_gpibController = new GpibControllerHardware();
	QThread *gpibThread = nullptr;
	d_hardwareList.append(qMakePair(p_gpibController,gpibThread));

	//Assuming AOM synth is a GPIB instrument...
	p_aomSynth = new AomSynthesizerHardware(p_gpibController);
	connect(p_aomSynth,&AomSynthesizer::frequencyUpdate,this,&HardwareManager::aomSynthUpdate);
	d_hardwareList.append(qMakePair(p_aomSynth,gpibThread));

	//freq comb might need thread, depending on how long network comms take (probably not)
	p_freqComb = new FreqCombHardware();
	connect(p_freqComb,&FrequencyComb::combUpdate,this,&HardwareManager::combUpdate);
	connect(p_freqComb,&FrequencyComb::repRateUpdate,this,&HardwareManager::repRateUpdate);
	connect(p_aomSynth,&AomSynthesizer::frequencyUpdate,p_freqComb,&FrequencyComb::setAOMFreq);
    connect(p_counter,&FrequencyCounter::freqUpdate,p_freqComb,&FrequencyComb::setCounterFreq);
	connect(this,&HardwareManager::setCombPumpBeat,p_freqComb,&FrequencyComb::setPumpBeat);
	connect(this,&HardwareManager::setCombSignalBeat,p_freqComb,&FrequencyComb::setSignalBeat);
	d_hardwareList.append(qMakePair(p_freqComb,nullptr));


	//write arrays of the connected devices for use in the Hardware Settings menu
	//first array is for all objects accessible to the hardware manager
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(QString("hardware"));
	s.remove("");
	s.beginWriteArray("instruments");
	for(int i=0;i<d_hardwareList.size();i++)
	{
		HardwareObject *obj = d_hardwareList.at(i).first;
		s.setArrayIndex(i);
		s.setValue(QString("key"),obj->key());
		s.setValue(QString("subKey"),obj->subKey());
		s.setValue(QString("prettyName"),obj->name());
		s.setValue(QString("critical"),obj->isCritical());
	}
	s.endArray();
	s.endGroup();

    //now an array for all TCP instruments
    s.beginGroup(QString("tcp"));
    s.remove("");
    s.beginWriteArray("instruments");
    int index=0;
    for(int i=0;i<d_hardwareList.size();i++)
    {
        if(d_hardwareList.at(i).first->type() == CommunicationProtocol::Tcp)
        {
            s.setArrayIndex(index);
            s.setValue(QString("key"),d_hardwareList.at(i).first->key());
            s.setValue(QString("subKey"),d_hardwareList.at(i).first->subKey());
            index++;
        }
    }
    s.endArray();
    s.endGroup();

    //now an array for all RS232 instruments
    s.beginGroup(QString("rs232"));
    s.remove("");
    s.beginWriteArray("instruments");
    index=0;
    for(int i=0;i<d_hardwareList.size();i++)
    {
        if(d_hardwareList.at(i).first->type() == CommunicationProtocol::Rs232)
        {
            s.setArrayIndex(index);
            s.setValue(QString("key"),d_hardwareList.at(i).first->key());
            s.setValue(QString("subKey"),d_hardwareList.at(i).first->subKey());
            index++;
        }
    }
    s.endArray();
    s.endGroup();

    //now an array for all GPIB instruments
    s.beginGroup(QString("gpib"));
    s.remove("");
    s.beginWriteArray("instruments");
    index=0;
    for(int i=0;i<d_hardwareList.size();i++)
    {
       if(d_hardwareList.at(i).first->type() == CommunicationProtocol::Gpib)
        {
            s.setArrayIndex(index);
            s.setValue(QString("key"),d_hardwareList.at(i).first->key());
            s.setValue(QString("subKey"),d_hardwareList.at(i).first->subKey());
            index++;
        }
    }
    s.endArray();
    s.endGroup();

	s.sync();

    for(int i=0;i<d_hardwareList.size();i++)
    {
        QThread *thread = d_hardwareList.at(i).second;
        HardwareObject *obj = d_hardwareList.at(i).first;

        s.setValue(QString("%1/prettyName").arg(obj->key()),obj->name());
	   s.setValue(QString("%1/subKey").arg(obj->key()),obj->subKey());
	   s.setValue(QString("%1/connected").arg(obj->key()),false);
	   s.setValue(QString("%1/critical").arg(obj->key()),obj->isCritical());

	   connect(obj,&HardwareObject::logMessage,[=](QString msg, NicerOhms::LogMessageCode mc){
            emit logMessage(QString("%1: %2").arg(obj->name()).arg(msg),mc);
        });
        connect(obj,&HardwareObject::connected,[=](bool success, QString msg){ connectionResult(obj,success,msg); });
        connect(obj,&HardwareObject::hardwareFailure,[=](bool abort){ hardwareFailure(obj,abort); });
	   connect(obj,&HardwareObject::pointDataRead,[=](QList<QPair<QString,QVariant>> l){ emit pointData(l,true);});
	   connect(obj,&HardwareObject::pointDataReadNoPlot,[=](QList<QPair<QString,QVariant>> l){ emit pointData(l,false);});
        connect(this,&HardwareManager::beginAcquisition,obj,&HardwareObject::beginAcquisition);
        connect(this,&HardwareManager::endAcquisition,obj,&HardwareObject::endAcquisition);
	   connect(this,&HardwareManager::readPointData,obj,&HardwareObject::readPointData);

        if(thread != nullptr)
        {
            connect(thread,&QThread::started,obj,&HardwareObject::initialize);
            connect(thread,&QThread::finished,obj,&HardwareObject::deleteLater);
            obj->moveToThread(thread);
        }
        else
            obj->initialize();
    }

    //now, start all threads
    for(int i=0;i<d_hardwareList.size();i++)
    {
	   QThread *thread = d_hardwareList.at(i).second;
	   if(thread != nullptr)
	   {
		   if(!thread->isRunning())
			   thread->start();
	   }
    }

    connect(this,&HardwareManager::endAcquisition,this,&HardwareManager::cleanUpAfterScan);
}

void HardwareManager::connectionResult(HardwareObject *obj, bool success, QString msg)
{
    if(success)
	    emit logMessage(obj->name().append(QString(": Connected successfully.")));
    else
    {
	    emit logMessage(obj->name().append(QString(": Failed to connect.")),NicerOhms::LogError);
	    if(obj->isCritical())
		    emit logMessage(msg,NicerOhms::LogError);
	    else
		    emit logMessage(msg,NicerOhms::LogWarning);
    }

    bool ok = success;
    if(!obj->isCritical())
        ok = true;

    if(d_status.contains(obj->key()))
        d_status[obj->key()] = ok;
    else
        d_status.insert(obj->key(),ok);

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    s.setValue(QString("%1/connected").arg(obj->key()),success);
    s.sync();

    emit testComplete(obj->name(),success,msg);
    checkStatus();
}

void HardwareManager::hardwareFailure(HardwareObject *obj, bool abort)
{
    if(abort)
        emit abortAcquisition();

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    s.setValue(QString("%1/connected").arg(obj->key()),false);
    s.sync();

    if(!obj->isCritical())
	   return;

    d_status[obj->key()] = false;
    checkStatus();
}

void HardwareManager::beginScanInitialization(Scan s)
{
    //initial synchronous initialization can go here
    //
    //


    //if the comb is active, we need to go into the wavemeter read procedure
    if(s.type() == Scan::CombScan)
    {
        //Add AOM lock parameters here
        //turn off deltaN override if it was left on
        setCombOverrideDN(-1);


        QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
        QString sk = set.value(QString("freqComb/subKey"),QString("virtual")).toString();

        int numReads = set.value(QString("freqComb/%1/wavemeterReads").arg(sk),10).toInt();



       //use wavemeterReadcontroller to get readings
        WavemeterReadController *wmr = new WavemeterReadController(numReads);
        emit statusMessage(QString("Collecting %1 wavemeter readings to determine mode numbers...").arg(wmr->targetReads()));
        connect(p_wavemeter,&Wavemeter::freqUpdate,wmr,&WavemeterReadController::readComplete);
        connect(this,&HardwareManager::uiAbort,wmr,&WavemeterReadController::abort);
        connect(wmr,&WavemeterReadController::readsComplete,[=](bool aborted) {
            disconnect(p_wavemeter,&Wavemeter::freqUpdate,wmr,&WavemeterReadController::readComplete);
            disconnect(this,&HardwareManager::abortAcquisition,wmr,&WavemeterReadController::abort);

            if(aborted)
                completeScanInitialization(s,false,QString("Could not make wavemeter readings before acquisition."));
            else
            {
                //note: can test wmr->stdev here

                completeScanInitialization(s);
            }
            wmr->deleteLater();

        });
    }
    else
        completeScanInitialization(s);
}

void HardwareManager::completeScanInitialization(Scan s, bool stageOneSuccess, QString errorMsg)
{
    //only go on to phase 2 if phase 1 was successful
    if(stageOneSuccess)
    {
        emit statusMessage(QString("Initializing hardware for scan..."));
        //do phase 2 initialization
        //if successful, call Scan::setInitialized()

        if(p_counter->read()<180e6)
        {
            double min  = p_counter->read() - 5e6;
            double max = min + 50e6;
            s.setPumpLockRange(min,max);
        }
        else
        {
            double max  = p_counter->read() + 5e6;
            double min = max - 50e6;
            s.setPumpLockRange(min, max);
        }


        bool success = true;
        for(int i=0;i<d_hardwareList.size();i++)
        {
            QThread *t = d_hardwareList.at(i).second;
            HardwareObject *obj = d_hardwareList.at(i).first;
            bool active = s.isHardwareActive(obj->key());

            if(t != nullptr)
            {
                QMetaObject::invokeMethod(obj,"setActive",Qt::BlockingQueuedConnection,Q_ARG(bool,active));
                QMetaObject::invokeMethod(obj,"prepareForScan",Qt::BlockingQueuedConnection,Q_RETURN_ARG(Scan,s),Q_ARG(Scan,s));
            }
            else
            {
                obj->setActive(active);
                s = obj->prepareForScan(s);
            }

            if(!s.hardwareSuccess())
            {
                success = false;
                break;
            }
        }

        //any final synchronous initialization can be performed here

        if(success)
            s.setInitialized();
    }
    else
    {
        s.setHardwareFailed();
        s.setErrorString(errorMsg);
    }

    if(s.type() == Scan::LaserScan)
        connect(p_laser,&Laser::slewComplete,this,&HardwareManager::readyForPoint,Qt::UniqueConnection);

    emit scanInitialized(s);
}

void HardwareManager::beginCombPoint(double shiftMHz)
{

    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    bool sigLock = set.value(QString("lastScanConfig/signalLock"),false).toBool();
    bool pumpLock = set.value(QString("lastScanConfig/pumpLock"),false).toBool();
    double laserStart = set.value(QString("lastScanConfig/laserStart"),20).toDouble();
    double MHzToV = set.value(QString("lastScanConfig/MHzToV"),.003).toDouble();

    //if the shift is 0, then nothing needs to be done

    if(qFuzzyCompare(1.0+shiftMHz,1.0))
    {
        emit readyForPoint();
		return;
    }

	FreqCombData d = getLastCombReading();

	//need to set rep rate, calculate what do do with aom and deltaN

	double pumpFreqEstimate = estimateLaserFrequency();
    int pumpModeEstimate = qRound(pumpFreqEstimate/d.repRate());
    int signalModeEstimate = pumpModeEstimate - d.deltaN();

    double repRateShift = shiftMHz*1e6/qRound(pumpFreqEstimate/d.repRate());


    //figure out if aom needs ratchet

    double nextAomFreq ;

    if(sigLock)
    {
        nextAomFreq = d.aomFreq()*1e6 - (shiftMHz*1e6)/2 + (30e6 - fabs(d.pumpBeat()))*p_freqComb->pumpSign()/2;
    }
    else
    {
        nextAomFreq = d.aomFreq()*1e6 + static_cast<double>(signalModeEstimate)*repRateShift/2.0 + p_freqComb->signalSign()*(30e6 - fabs(d.signalBeat()))/2;//CRM: Corrected for difference of sbeat to 30 MHz to correct for initial offset and hysterisis in pump estimation. Also, aomFreq() needed to be multiplied by 10^6
    }

    double lt = aomLowTrip();
    double ht = aomHighTrip();

    //since we're not updating the comb's knowledge of the idler frequency, we need to tell it the mode number difference

    if(nextAomFreq > ht)
    {

        nextAomFreq -= 50e6;

        if(nextAomFreq<lt||nextAomFreq>ht)
            nextAomFreq +=50e6;
        else
        {
            if(sigLock)
            {
                if(!pumpLock)
                {
                    setCombOverrideDN(d.deltaN()-1);
                    qDebug() << "ht siglock !pumplock" ;
                }
            }
            else
            {
                if(!pumpLock)
                {
                setCombOverrideDN(d.deltaN()+1);
                qDebug() << "ht !siglock !pumplock" ;
                }
            }
        }
    }
    else if(nextAomFreq < lt)
    {

        nextAomFreq += 50e6;

        if(nextAomFreq<lt||nextAomFreq>ht)
            nextAomFreq -=50e6;
        else
        {
            if(sigLock)
            {
                if(!pumpLock)
                {
                    setCombOverrideDN(d.deltaN()+1);
                    qDebug() << "lt siglock !pumplock" ;
                }
            }
            else
            {
                if(!pumpLock)
                {
                    setCombOverrideDN(d.deltaN()-1);
                    qDebug() << "lt !siglock !pumplock" ;
                }
            }
        }
    }
    else
    {
        if(!pumpLock)
            setCombOverrideDN(d.deltaN());
    }

    if(sigLock)
    {
        laserStart += shiftMHz*MHzToV;
        slewLaser(laserStart);
        set.setValue("lastScanConfig/laserStart",laserStart);
    }
    else
    {
        setCombRepRate(d.repRate() + repRateShift);
    }

    if(!pumpLock)
    {
        setAomFrequency(nextAomFreq);
    }
    else
    {

        if(d.counterFreq()>ht)
        {
//            setCombOverrideDN(d.deltaN()-1);
            qDebug() << "ht" ;
//            emit pauseAcquisition();
        }
        else if(d.counterFreq()<lt)
        {
//            setCombOverrideDN(d.deltaN()+1);
            qDebug() << "lt" ;
//            emit pauseAcquisition();
        }
    }


    emit readyForPoint();
}

void HardwareManager::testObjectConnection(const QString type, const QString key)
{
    Q_UNUSED(type)
    HardwareObject *obj = nullptr;
    for(int i=0; i<d_hardwareList.size();i++)
    {
        if(d_hardwareList.at(i).first->key() == key)
            obj = d_hardwareList.at(i).first;
    }
    if(obj == nullptr)
        emit testComplete(key,false,QString("Device not found!"));
    else
	    QMetaObject::invokeMethod(obj,"testConnection");
}

void HardwareManager::testAllConnections()
{
    for(int i=0; i<d_hardwareList.size(); i++)
    {
        d_status[d_hardwareList.at(i).first->key()] = false;
        QMetaObject::invokeMethod(d_hardwareList.at(i).first,"testConnection");
    }

    checkStatus();
}

void HardwareManager::getPointData()
{
    for(int i=0; i<d_hardwareList.size(); i++)
        QMetaObject::invokeMethod(d_hardwareList.at(i).first,"readPointData");
}

void HardwareManager::cleanUpAfterScan()
{
    connect(p_laser,&Laser::slewComplete,this,&HardwareManager::readyForPoint);
}

double HardwareManager::estimateLaserFrequency()
{
	if(p_laser->thread() == thread())
		return p_laser->estimateFrequency();
	else
	{
		double out;
		QMetaObject::invokeMethod(p_laser,"estimateFrequency",Qt::BlockingQueuedConnection,Q_RETURN_ARG(double,out));
		return out;
	}
}

void HardwareManager::checkLock(bool pztEnabled, bool pumpLockEnabled)
{

	bool locked = false;

	if(p_iob->thread() == thread())
    {
        if(pztEnabled)
            locked = p_iob->readCavityLocked();
        else
            locked = true;

    }
	else
		QMetaObject::invokeMethod(p_iob,"readCavityLocked",Qt::BlockingQueuedConnection,Q_RETURN_ARG(bool,locked));

    if(!locked || !pztEnabled)
    {
        if(pumpLockEnabled)
        {
            double f = getCounterFrequency();
            emit lockStateCheck(locked,-1.0,f);
        }
        else
            emit lockStateCheck(locked,-1.0,-1.0);
    }
	else
	{
        if(pumpLockEnabled)
        {
            double v = checkCavityVoltage();
            double f = getCounterFrequency();
            emit lockStateCheck(locked,v,f);
        }
        else
        {
            double v = checkCavityVoltage();
            emit lockStateCheck(locked,v,-1.0);
        }
    }

}

double HardwareManager::checkCavityVoltage()
{
	double out = 0.0;
	if(p_cavityPZT->thread() == thread())
		out = p_cavityPZT->readVoltage();
	else
		QMetaObject::invokeMethod(p_iob,"readVoltage",Qt::BlockingQueuedConnection,Q_RETURN_ARG(double,out));

	return out;
}

double HardwareManager::getCounterFrequency()
{
    double out = 0.0;
    if(p_counter->thread() == thread())
        out = p_counter->read();
    else
        QMetaObject::invokeMethod(p_counter,"read",Qt::BlockingQueuedConnection,Q_RETURN_ARG(double,out));
}

double HardwareManager::getAomFrequency()
{
	if(p_aomSynth->thread() == thread())
		return p_aomSynth->getLastFrequency();
	else
	{
		double out;
		QMetaObject::invokeMethod(p_aomSynth,"getLastFrequency",Qt::BlockingQueuedConnection,Q_RETURN_ARG(double,out));
		return out;
	}
}

void HardwareManager::setAomFrequency(double f)
{
	if(p_aomSynth->thread() == thread())
		p_aomSynth->setFrequency(f);
	else
        QMetaObject::invokeMethod(p_aomSynth,"setFrequency",Qt::BlockingQueuedConnection,Q_ARG(double,f));
}

double HardwareManager::aomLowTrip()
{
    if(p_aomSynth->thread() == thread())
        return p_aomSynth->lowTrip();
    else
    {
        double out;
        QMetaObject::invokeMethod(p_aomSynth,"lowTrip",Qt::BlockingQueuedConnection,Q_RETURN_ARG(double,out));
        return out;
    }
}

double HardwareManager::aomHighTrip()
{
    if(p_aomSynth->thread() == thread())
        return p_aomSynth->highTrip();
    else
    {
        double out;
        QMetaObject::invokeMethod(p_aomSynth,"highTrip",Qt::BlockingQueuedConnection,Q_RETURN_ARG(double,out));
        return out;
    }
}

void HardwareManager::readComb()
{
	//use wavemeterReadcontroller to get readings (TODO: use settings to get target reads...)
    //turn off deltaN override if it was left on
    setCombOverrideDN(-1);

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    QString sk = s.value(QString("freqComb/subKey"),QString("virtual")).toString();
    int numReads = s.value(QString("freqComb/%1/wavemeterReads").arg(sk),10).toInt();

    WavemeterReadController *wmr = new WavemeterReadController(numReads);
    emit statusMessage(QString("Collecting %1 wavemeter readings to determine mode numbers...").arg(wmr->targetReads()));
	connect(p_wavemeter,&Wavemeter::freqUpdate,wmr,&WavemeterReadController::readComplete);
    connect(this,&HardwareManager::uiAbort,wmr,&WavemeterReadController::abort);
	connect(wmr,&WavemeterReadController::readsComplete,[=](bool aborted) {
	    disconnect(p_wavemeter,&Wavemeter::freqUpdate,wmr,&WavemeterReadController::readComplete);
	    disconnect(this,&HardwareManager::abortAcquisition,wmr,&WavemeterReadController::abort);

	    if(!aborted)
	    {
		    //NOTE: can cancel read if stdev too high!
		    setCombIdlerFreq(wmr->freqMean());
		    QMetaObject::invokeMethod(p_freqComb,"readComb");
	    }
	    else
        {
            FreqCombData out;
            out.setSuccess(false);
            emit combUpdate(out);
        }
	    wmr->deleteLater();

	});
}

void HardwareManager::setCombRepRate(double f)
{
	if(p_freqComb->thread() == thread())
		p_freqComb->setRepRate(f);
	else
		QMetaObject::invokeMethod(p_freqComb,"setRepRate",Qt::BlockingQueuedConnection,Q_ARG(double,f));
}

void HardwareManager::setCombIdlerFreq(double f)
{
	if(p_freqComb->thread() == thread())
		p_freqComb->setIdlerFreq(f);
	else
		QMetaObject::invokeMethod(p_freqComb,"setIdlerFreq",Q_ARG(double,f));
}

void HardwareManager::setCombOverrideDN(int dN)
{
    qDebug() << "changing dN to " << dN;
	if(p_freqComb->thread() == thread())
		p_freqComb->setDeltaNOverride(dN);
	else
		QMetaObject::invokeMethod(p_freqComb,"setDeltaNOverride",Qt::BlockingQueuedConnection,Q_ARG(int,dN));
}

FreqCombData HardwareManager::getLastCombReading()
{
	if(p_freqComb->thread() == thread())
		return p_freqComb->getLastMeasurement();
	else
	{
		FreqCombData out;
		QMetaObject::invokeMethod(p_freqComb,"getLastMeasurement",Qt::BlockingQueuedConnection,Q_RETURN_ARG(FreqCombData,out));
		return out;
	}
}

void HardwareManager::checkStatus()
{
    //gotta wait until all instruments have responded
    if(d_status.size() < d_hardwareList.size())
        return;

    bool success = true;
    foreach (bool b, d_status)
    {
        if(!b)
            success = false;
    }

    emit allHardwareConnected(success);
}

void HardwareManager::manualPumpRelockCheck(bool abort, bool tripH)
{
    qDebug() << "hwm manPumprelockcheck";
    FreqCombData d = getLastCombReading();
    if(tripH)
        setCombOverrideDN(d.deltaN()-1);
    else
        setCombOverrideDN(d.deltaN()+1);


    emit manualPumpRelockComp(abort);



}

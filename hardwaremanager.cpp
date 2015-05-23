#include "hardwaremanager.h"

#include <QSettings>

#include "hardwareobject.h"

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
            p.second->quit();
            p.second->wait();
        }
        else
            p.first->deleteLater();
    }
}

void HardwareManager::initialize()
{    
	//write arrays of the connected devices for use in the Hardware Settings menu
	//first array is for all objects accessible to the hardware manager
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(QString("hardware"));
	s.remove("");
	s.beginWriteArray("instruments");
	for(int i=0;i<d_hardwareList.size();i++)
	{
		s.setArrayIndex(i);
		s.setValue(QString("key"),d_hardwareList.at(i).first->key());
        s.setValue(QString("type"),d_hardwareList.at(i).first->subKey());
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

	   connect(obj,&HardwareObject::logMessage,[=](QString msg, NicerOhms::LogMessageCode mc){
            emit logMessage(QString("%1: %2").arg(obj->name()).arg(msg),mc);
        });
        connect(obj,&HardwareObject::connected,[=](bool success, QString msg){ connectionResult(obj,success,msg); });
        connect(obj,&HardwareObject::hardwareFailure,[=](bool abort){ hardwareFailure(obj,abort); });
	   connect(obj,&HardwareObject::pointDataRead,this,&HardwareManager::pointData);
        connect(this,&HardwareManager::beginAcquisition,obj,&HardwareObject::beginAcquisition);
        connect(this,&HardwareManager::endAcquisition,obj,&HardwareObject::endAcquisition);
	   connect(this,&HardwareManager::readPointData,obj,&HardwareObject::readPointData);

        if(thread != nullptr)
        {
            connect(thread,&QThread::started,obj,&HardwareObject::initialize);
            connect(thread,&QThread::finished,obj,&HardwareObject::deleteLater);
            obj->moveToThread(thread);
            thread->start();
        }
        else
            obj->initialize();

    }
}

void HardwareManager::connectionResult(HardwareObject *obj, bool success, QString msg)
{
    if(success)
        emit logMessage(obj->name().append(QString(" connected successfully.")));
    else
    {
	   emit logMessage(obj->name().append(QString(" connection failed!")),NicerOhms::LogError);
	   emit logMessage(msg,NicerOhms::LogError);
    }

    bool ok = success;
    if(!obj->isCritical())
        ok = true;

    if(d_status.contains(obj->key()))
        d_status[obj->key()] = ok;
    else
        d_status.insert(obj->key(),ok);



    emit testComplete(obj->name(),success,msg);
    checkStatus();
}

void HardwareManager::hardwareFailure(HardwareObject *obj, bool abort)
{
    if(abort)
        emit abortAcquisition();

    if(!obj->isCritical())
        return;

    d_status[obj->key()] = false;
    checkStatus();
}

void HardwareManager::initializeScan(Scan s)
{
    //do initialization
    //if successful, call Scan::setInitialized()
    bool success = true;
    for(int i=0;i<d_hardwareList.size();i++)
    {
        QThread *t = d_hardwareList.at(i).second;
        HardwareObject *obj = d_hardwareList.at(i).first;
        if(t != nullptr)
		  QMetaObject::invokeMethod(obj,"prepareForScan",Qt::BlockingQueuedConnection,Q_RETURN_ARG(Scan,s),Q_ARG(Scan,s));
        else
		  s = obj->prepareForScan(s);

	   if(!s.hardwareSuccess())
        {
            success = false;
            break;
        }
    }

    //any additional synchronous initialization can be performed here

    if(success)
	   s.setInitialized();

    emit scanInitialized(s);

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

void HardwareManager::getPointData()
{
    for(int i=0; i<d_hardwareList.size(); i++)
	   QMetaObject::invokeMethod(d_hardwareList.at(i).first,"readPointData");
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

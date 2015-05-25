#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QLabel>
#include <QMessageBox>

#include "loghandler.h"
#include "communicationdialog.h"
#include "hardwaremanager.h"
#include "acquisitionmanager.h"
#include "batchmanager.h"

#include "batchsingle.h"

MainWindow::MainWindow(QWidget *parent) :
     QMainWindow(parent),
	ui(new Ui::MainWindow), d_hardwareConnected(false)
{
	ui->setupUi(this);

	QLabel *statusLabel = new QLabel(this);
	connect(this,&MainWindow::statusMessage,statusLabel,&QLabel::setText);
	ui->statusBar->addWidget(statusLabel);

	p_lh = new LogHandler();
	connect(p_lh,&LogHandler::sendLogMessage,ui->logTextEdit,&QTextEdit::append);

	QThread *lhThread = new QThread(this);
	connect(lhThread,&QThread::finished,p_lh,&LogHandler::deleteLater);
	p_lh->moveToThread(lhThread);
	d_threadObjectList.append(qMakePair(lhThread,p_lh));
	lhThread->start();

	p_hwm = new HardwareManager;
	connect(p_hwm,&HardwareManager::logMessage,p_lh,&LogHandler::logMessage);
	connect(p_hwm,&HardwareManager::statusMessage,statusLabel,&QLabel::setText);
	connect(p_hwm,&HardwareManager::scanInitialized,this,&MainWindow::scanInitialized);

	QThread *hwmThread = new QThread(this);
	connect(hwmThread,&QThread::started,p_hwm,&HardwareManager::initialize);
	connect(hwmThread,&QThread::finished,p_hwm,&HardwareManager::deleteLater);
	p_hwm->moveToThread(hwmThread);
	d_threadObjectList.append(qMakePair(hwmThread,p_hwm));

	p_am = new AcquisitionManager;
	connect(p_am,&AcquisitionManager::logMessage,p_lh,&LogHandler::logMessage);
	connect(p_am,&AcquisitionManager::statusMessage,statusLabel,&QLabel::setText);
	connect(p_am,&AcquisitionManager::pointComplete,ui->scanProgressBar,&QProgressBar::setValue);
	connect(p_am,&AcquisitionManager::requestManualLock,this,&MainWindow::manualRelock);
	connect(this,&MainWindow::manualRelockComplete,p_am,&AcquisitionManager::manualLockComplete);
	connect(p_am,&AcquisitionManager::beginAcquisition,p_hwm,&HardwareManager::beginAcquisition);
	connect(p_am,&AcquisitionManager::startPoint,p_hwm,&HardwareManager::slewLaser);
	connect(p_am,&AcquisitionManager::checkLock,p_hwm,&HardwareManager::checkLock);
	connect(p_am,&AcquisitionManager::getPointData,p_hwm,&HardwareManager::getPointData);
	connect(p_am,&AcquisitionManager::scanComplete,p_hwm,&HardwareManager::endAcquisition);
	connect(p_am,&AcquisitionManager::requestAutoLock,p_hwm,&HardwareManager::autoRelock);
	connect(p_hwm,&HardwareManager::scanInitialized,p_am,&AcquisitionManager::beginScan);
	connect(p_hwm,&HardwareManager::lockStateCheck,p_am,&AcquisitionManager::lockCheckComplete);
	connect(p_hwm,&HardwareManager::pointData,p_am,&AcquisitionManager::processData);
	connect(p_hwm,&HardwareManager::relockComplete,p_am,&AcquisitionManager::autoLockComplete);

	QThread *amThread = new QThread(this);
	connect(amThread,&QThread::started,p_am,&AcquisitionManager::initialize);
	connect(amThread,&QThread::finished,p_am,&AcquisitionManager::deleteLater);
	p_am->moveToThread(amThread);
	d_threadObjectList.append(qMakePair(amThread,p_am));

	connect(ui->actionCommunication,&QAction::triggered,this,&MainWindow::launchCommunicationDialog);
	connect(ui->actionTest,&QAction::triggered,this,&MainWindow::test);

	p_batchThread = new QThread(this);

	hwmThread->start();
	amThread->start();
}

MainWindow::~MainWindow()
{
	while(!d_threadObjectList.isEmpty())
	{
		QPair<QThread*,QObject*> p = d_threadObjectList.takeFirst();

		p.first->quit();
		p.first->wait();
	}

	delete ui;
}

void MainWindow::launchCommunicationDialog()
{
	CommunicationDialog d(this);
	connect(&d,&CommunicationDialog::testConnection,p_hwm,&HardwareManager::testObjectConnection);
	connect(p_hwm,&HardwareManager::testComplete,&d,&CommunicationDialog::testComplete);

	d.exec();
}

void MainWindow::hardwareConnected(bool connected)
{
	d_hardwareConnected = connected;
	configureUi(d_currentState);
}

void MainWindow::manualRelock()
{
	int ret = QMessageBox::question(this,QString("Manual Relock"),QString("Laser-cavity lock has been lost. Press ok when the lock is restored, or press abort to terminate the scan."),QMessageBox::Ok|QMessageBox::Abort, QMessageBox::Ok);

	if(ret == QMessageBox::Abort)
		emit manualRelockComplete(true);
	else
		emit manualRelockComplete(false);
}

void MainWindow::scanInitialized(const Scan s)
{
	ui->scanProgressBar->setValue(0);
	ui->scanProgressBar->setMaximum(s.totalPoints());
}

void MainWindow::batchComplete(bool aborted)
{
	disconnect(p_hwm,&HardwareManager::lockStateUpdate,p_am,&AcquisitionManager::lockStateUpdate);
	disconnect(p_am,&AcquisitionManager::plotData,ui->dataPlotWidget,&DataPlotViewWidget::pointUpdated);
	disconnect(p_hwm,&HardwareManager::laserSlewComplete,p_am,&AcquisitionManager::laserReady);

	if(aborted)
	    emit statusMessage(QString("Scan aborted"));
	else
	    emit statusMessage(QString("Scan complete"));

	configureUi(Idle);

}

void MainWindow::test()
{
	Scan s;
	s.setLaserParams(100.0,1.0,5.1,50);

	BatchManager *bm = new BatchSingle(s);
	beginBatch(bm);
}

void MainWindow::beginBatch(BatchManager *bm)
{
	connect(p_batchThread,&QThread::started,bm,&BatchManager::beginNextScan);
	connect(bm,&BatchManager::logMessage,p_lh,&LogHandler::logMessage);
	connect(bm,&BatchManager::beginScan,p_hwm,&HardwareManager::initializeScan);
	connect(p_am,&AcquisitionManager::scanComplete,bm,&BatchManager::scanComplete);
	connect(bm,&BatchManager::batchComplete,this,&MainWindow::batchComplete);
	connect(bm,&BatchManager::batchComplete,p_batchThread,&QThread::quit);
	connect(p_batchThread,&QThread::finished,bm,&BatchManager::deleteLater);

	connect(p_hwm,&HardwareManager::laserSlewComplete,p_am,&AcquisitionManager::laserReady,Qt::UniqueConnection);
	connect(p_hwm,&HardwareManager::lockStateUpdate,p_am,&AcquisitionManager::lockStateUpdate,Qt::UniqueConnection);
	connect(p_am,&AcquisitionManager::plotData,ui->dataPlotWidget,&DataPlotViewWidget::pointUpdated,Qt::UniqueConnection);

//	if(sleepWhenDone)
//	{
//		//connect to sleep action
//	}

	ui->dataPlotWidget->initializeForExperiment();
	configureUi(Acquiring);
	bm->moveToThread(p_batchThread);
	p_batchThread->start();

}

void MainWindow::configureUi(MainWindow::UiState s)
{
	if(!d_hardwareConnected)
		s = Disconnected;

	switch(s)
	{
	case Acquiring:
		break;
	case Slewing:
		break;
	case Disconnected:
		break;
	case Asleep:
		break;
	case Idle:
	default:
		break;
	}

	if(s != Disconnected)
		d_currentState = s;
}

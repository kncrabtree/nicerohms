#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QActionGroup>

#include "loghandler.h"
#include "communicationdialog.h"
#include "hardwaremanager.h"
#include "acquisitionmanager.h"
#include "batchmanager.h"

#include "batchsingle.h"

MainWindow::MainWindow(QWidget *parent) :
     QMainWindow(parent),
	ui(new Ui::MainWindow), d_currentState(Idle), d_hardwareConnected(false)
{
	ui->setupUi(this);

	QLabel *statusLabel = new QLabel(this);
	statusLabel->setText(QString("Waiting for hardware..."));
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
	connect(p_hwm,&HardwareManager::allHardwareConnected,this,&MainWindow::hardwareConnected);
	connect(p_hwm,&HardwareManager::scanInitialized,this,&MainWindow::scanInitialized);
	connect(p_hwm,&HardwareManager::laserPosUpdate,ui->laserDoubleSpinBox,&QDoubleSpinBox::setValue);
	connect(p_hwm,&HardwareManager::lockStateUpdate,ui->lockLed,&Led::setState);
	connect(p_hwm,&HardwareManager::lockStateCheck,ui->lockLed,&Led::setState);
	connect(p_hwm,&HardwareManager::wavemeterPumpUpdate,this,&MainWindow::pumpUpdate);
	connect(p_hwm,&HardwareManager::wavemeterSignalUpdate,this,&MainWindow::signalUpdate);

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

	ui->scanNumberSpinBox->blockSignals(true);
	ui->laserDoubleSpinBox->blockSignals(true);
	ui->pumpDoubleSpinBox->blockSignals(true);
	ui->signalDoubleSpinBox->blockSignals(true);
	ui->idlerDoubleSpinBox->blockSignals(true);

	readWavemeterSettings();
	QActionGroup *wmActions = new QActionGroup(this);
	wmActions->addAction(ui->actionGHz);
	wmActions->addAction(ui->actionWavenumbers);
	wmActions->setExclusive(true);
	switch (d_wmUnits) {
	case NicerOhms::WmWavenumbers:
		ui->actionWavenumbers->setChecked(true);
		break;
	case NicerOhms::WmGHz:
		ui->actionGHz->setChecked(true);
		break;
	}

	connect(ui->actionGHz,&QAction::triggered,[=](){ setWavemeterUnits(NicerOhms::WmGHz); });
	connect(ui->actionWavenumbers,&QAction::triggered,[=](){ setWavemeterUnits(NicerOhms::WmWavenumbers); });

	ui->menuUnits->addAction(ui->actionGHz);
	ui->menuUnits->addAction(ui->actionWavenumbers);

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
	if(connected)
		emit statusMessage(QString("Hardware connected successfully."));
	else
		emit statusMessage(QString("Hardware is not all connected."));
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
	ui->scanNumberSpinBox->setValue(s.number());
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
	s.setLaserParams(100.0,1.0,5.1,500);

	BatchManager *bm = new BatchSingle(s);
	beginBatch(bm);
}

void MainWindow::setWavemeterUnits(NicerOhms::WmDisplayUnits u)
{
	QSettings s(QSettings::SystemScope, QApplication::organizationName(), QApplication::applicationName());
	s.setValue(QString("wmUnits"),u);
	s.sync();
	readWavemeterSettings();
}

void MainWindow::readWavemeterSettings()
{
	QSettings s(QSettings::SystemScope, QApplication::organizationName(), QApplication::applicationName());
	NicerOhms::WmDisplayUnits units = static_cast<NicerOhms::WmDisplayUnits>
			(s.value(QString("wmUnits"),NicerOhms::WmWavenumbers).toInt());

	switch(units)
	{
	case NicerOhms::WmWavenumbers:
		ui->pumpDoubleSpinBox->setSuffix(QString::fromUtf16(u" cm⁻¹"));
		ui->signalDoubleSpinBox->setSuffix(QString::fromUtf16(u" cm⁻¹"));
		ui->idlerDoubleSpinBox->setSuffix(QString::fromUtf16(u" cm⁻¹"));
		ui->pumpDoubleSpinBox->setDecimals(6);
		ui->signalDoubleSpinBox->setDecimals(6);
		ui->idlerDoubleSpinBox->setDecimals(6);
		if(d_wmUnits == NicerOhms::WmGHz)
		{
			double newPump = ui->pumpDoubleSpinBox->value()/29.9792458;
			double newSignal = ui->signalDoubleSpinBox->value()/29.9792458;
			double newIdler = ui->idlerDoubleSpinBox->value()/29.9792458;
			ui->pumpDoubleSpinBox->setMaximum(10000.0);
			ui->pumpDoubleSpinBox->setValue(newPump);
			ui->signalDoubleSpinBox->setMaximum(10000.0);
			ui->signalDoubleSpinBox->setValue(newSignal);
			ui->idlerDoubleSpinBox->setMaximum(10000.0);
			ui->idlerDoubleSpinBox->setValue(newIdler);
		}
		break;
	case NicerOhms::WmGHz:
		ui->pumpDoubleSpinBox->setSuffix(QString(" GHz"));
		ui->signalDoubleSpinBox->setSuffix(QString(" GHz"));
		ui->idlerDoubleSpinBox->setSuffix(QString(" GHz"));
		ui->pumpDoubleSpinBox->setDecimals(5);
		ui->signalDoubleSpinBox->setDecimals(5);
		ui->idlerDoubleSpinBox->setDecimals(5);
		if(d_wmUnits == NicerOhms::WmWavenumbers)
		{
			double newPump = ui->pumpDoubleSpinBox->value()*29.9792458;
			double newSignal = ui->signalDoubleSpinBox->value()*29.9792458;
			double newIdler = ui->idlerDoubleSpinBox->value()*29.9792458;
			ui->pumpDoubleSpinBox->setMaximum(1000000.0);
			ui->pumpDoubleSpinBox->setValue(newPump);
			ui->signalDoubleSpinBox->setMaximum(1000000.0);
			ui->signalDoubleSpinBox->setValue(newSignal);
			ui->idlerDoubleSpinBox->setMaximum(1000000.0);
			ui->idlerDoubleSpinBox->setValue(newIdler);
		}
		break;
	}

	d_wmUnits = units;
}

void MainWindow::pumpUpdate(double f)
{
	double freq;
	switch (d_wmUnits) {
	case NicerOhms::WmWavenumbers:
		freq = f/29979245800.0;
		break;
	case NicerOhms::WmGHz:
		freq = f/1e9;
		break;
	}

	ui->pumpDoubleSpinBox->setValue(freq);
	if(!qFuzzyCompare(1.0 + ui->signalDoubleSpinBox->value(), 1.0))
		ui->idlerDoubleSpinBox->setValue(freq - ui->signalDoubleSpinBox->value());
}

void MainWindow::signalUpdate(double f)
{
	double freq;
	switch (d_wmUnits) {
	case NicerOhms::WmWavenumbers:
		freq = f/29979245800.0;
		break;
	case NicerOhms::WmGHz:
		freq = f/1e9;
		break;
	}

	ui->signalDoubleSpinBox->setValue(freq);
	if(ui->pumpDoubleSpinBox->value() - freq > 0.0)
		ui->idlerDoubleSpinBox->setValue(ui->pumpDoubleSpinBox->value() - freq);
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
	case Idle:
	default:
		break;
	}

	if(s != Disconnected)
		d_currentState = s;
}

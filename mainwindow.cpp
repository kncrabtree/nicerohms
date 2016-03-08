#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QActionGroup>
#include <QCloseEvent>
#include <QWidgetAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include "loghandler.h"
#include "communicationdialog.h"
#include "hardwaremanager.h"
#include "acquisitionmanager.h"
#include "batchmanager.h"
#include "laserslewaction.h"
#include "readcombaction.h"
#include "scanconfigwidget.h"
#include "ioboardconfigwidget.h"
#include "freqcombconfigwidget.h"

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
    connect(p_hwm,&HardwareManager::wavemeterFreqUpdate,[=](double f){ ui->idlerDoubleSpinBox->setValue(f/1e9);});
    connect(p_hwm,&HardwareManager::wavemeterFreqUpdate,[=](double f){ ui->idlerWNDoubleSpinBox->setValue(f/1e9/29.9792458); });
    connect(p_hwm,&HardwareManager::laserSlewStarted,[=](){ configForSlew(true);} );
    connect(p_hwm,&HardwareManager::laserSlewComplete,[=](){ configForSlew(false);} );
	connect(p_hwm,&HardwareManager::aomSynthUpdate,[=](double f){ ui->aomDoubleSpinBox->setValue(f/1e6);});
    connect(p_hwm,&HardwareManager::cavityPZTUpdate,ui->cPZTDoubleSpinBox,&QDoubleSpinBox::setValue);
	connect(p_hwm,&HardwareManager::repRateUpdate,this,&MainWindow::repRateUpdate);
	connect(p_hwm,&HardwareManager::combUpdate,this,&MainWindow::combUpdate);

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
	connect(p_am,&AcquisitionManager::startLaserPoint,p_hwm,&HardwareManager::slewLaser);
    connect(p_am,&AcquisitionManager::startCombPoint,p_hwm,&HardwareManager::beginCombPoint);
	connect(p_am,&AcquisitionManager::checkLock,p_hwm,&HardwareManager::checkLock);
	connect(p_am,&AcquisitionManager::getPointData,p_hwm,&HardwareManager::getPointData);
	connect(p_am,&AcquisitionManager::scanComplete,p_hwm,&HardwareManager::endAcquisition);
	connect(p_am,&AcquisitionManager::requestAutoLock,p_hwm,&HardwareManager::autoRelock);
	connect(p_hwm,&HardwareManager::scanInitialized,p_am,&AcquisitionManager::beginScan);
	connect(p_hwm,&HardwareManager::lockStateCheck,p_am,&AcquisitionManager::lockCheckComplete);
	connect(p_hwm,&HardwareManager::pointData,p_am,&AcquisitionManager::processData);
	connect(p_hwm,&HardwareManager::relockComplete,p_am,&AcquisitionManager::autoLockComplete);
	connect(p_am,&AcquisitionManager::startCombPoint,p_hwm,&HardwareManager::beginCombPoint);
    connect(p_hwm,&HardwareManager::readyForPoint,p_am,&AcquisitionManager::frequencyReady);

	QThread *amThread = new QThread(this);
	connect(amThread,&QThread::started,p_am,&AcquisitionManager::initialize);
	connect(amThread,&QThread::finished,p_am,&AcquisitionManager::deleteLater);
	p_am->moveToThread(amThread);
	d_threadObjectList.append(qMakePair(amThread,p_am));

    p_laserSlewAction = new LaserSlewAction(this);
    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    ui->mainToolBar->addWidget(spacer);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(p_laserSlewAction);
    ui->mainToolBar->addSeparator();
    p_readCombAction = new ReadCombAction(this);
    ui->mainToolBar->addAction(p_readCombAction);

	connect(ui->actionCommunication,&QAction::triggered,this,&MainWindow::launchCommunicationDialog);
    connect(ui->actionAbort,&QAction::triggered,p_hwm,&HardwareManager::uiAbort);
    connect(ui->actionStart_Laser_Scan,&QAction::triggered,[=](){ startScan(Scan::LaserScan);} );
    connect(ui->actionStart_Comb_Scan,&QAction::triggered,[=](){ startScan(Scan::CombScan);} );
	connect(ui->actionAbort,&QAction::triggered,p_am,&AcquisitionManager::abortScan);
	connect(ui->actionTest_All_Connections,&QAction::triggered,p_hwm,&HardwareManager::testAllConnections);
    connect(p_laserSlewAction,&LaserSlewAction::slewSignal,p_hwm,&HardwareManager::slewLaser);
    connect(p_readCombAction,&QAction::triggered,[=](){ configureUi(CombReading); });
    connect(p_readCombAction,&QAction::triggered,p_hwm,&HardwareManager::readComb);
    connect(p_readCombAction,&ReadCombAction::setPumpSign,p_hwm,&HardwareManager::setCombPumpBeat);
    connect(p_readCombAction,&ReadCombAction::setSignalSign,p_hwm,&HardwareManager::setCombSignalBeat);
    connect(ui->actionIOBoard,&QAction::triggered,this,&MainWindow::launchIOBoardDialog);
    connect(ui->actionFrequency_Comb,&QAction::triggered,this,&MainWindow::launchFreqCombDialog);
    connect(ui->actionNum_Data_Plots,&QAction::triggered,ui->dataPlotWidget,&DataPlotViewWidget::changeNumPlots);

	p_batchThread = new QThread(this);

	ui->scanNumberSpinBox->blockSignals(true);
	ui->laserDoubleSpinBox->blockSignals(true);
    ui->idlerDoubleSpinBox->blockSignals(true);
    ui->idlerWNDoubleSpinBox->blockSignals(true);
    ui->aomDoubleSpinBox->blockSignals(true);

	hwmThread->start();
	amThread->start();

    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    ui->scanNumberSpinBox->setValue(s.value(QString("scanNum"),1).toInt()-1);

	configureUi(Idle);
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

void MainWindow::launchIOBoardDialog()
{
	QDialog d(this);
    d.setWindowTitle(QString("IO Board Configuration"));
	IOBoardConfigWidget *io = new IOBoardConfigWidget(&d);
	QVBoxLayout *vbl = new QVBoxLayout;
	QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,&d);

	connect(io,&IOBoardConfigWidget::testConnection,p_hwm,&HardwareManager::testObjectConnection);
	connect(p_hwm,&HardwareManager::testComplete,io,&IOBoardConfigWidget::testComplete);

	connect(bb->button(QDialogButtonBox::Ok),&QPushButton::clicked,io,&IOBoardConfigWidget::saveToSettings);
	connect(bb->button(QDialogButtonBox::Ok),&QPushButton::clicked,&d,&QDialog::accept);
	connect(bb->button(QDialogButtonBox::Cancel),&QPushButton::clicked,&d,&QDialog::reject);

	vbl->addWidget(io,1);
	vbl->addWidget(bb,0);

	d.setLayout(vbl);
	d.exec();

}

void MainWindow::launchFreqCombDialog()
{
    QDialog d(this);
    d.setWindowTitle(QString("Frequency Comb Configuration"));
    FreqCombConfigWidget *fc = new FreqCombConfigWidget(&d);
    QVBoxLayout *vbl = new QVBoxLayout;
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,&d);

    connect(fc,&FreqCombConfigWidget::testConnection,p_hwm,&HardwareManager::testObjectConnection);
    connect(p_hwm,&HardwareManager::testComplete,fc,&FreqCombConfigWidget::testComplete);

    connect(bb->button(QDialogButtonBox::Ok),&QPushButton::clicked,fc,&FreqCombConfigWidget::saveToSettings);
    connect(bb->button(QDialogButtonBox::Ok),&QPushButton::clicked,&d,&QDialog::accept);
    connect(bb->button(QDialogButtonBox::Cancel),&QPushButton::clicked,&d,&QDialog::reject);

    vbl->addWidget(fc,1);
    vbl->addWidget(bb,0);

    d.setLayout(vbl);
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

void MainWindow::configForSlew(bool slewing)
{
	ui->slewLed->setState(slewing);
	if(p_batchThread->isRunning())
		return;

	if(slewing)
		configureUi(Slewing);
	else
		configureUi(Idle);
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
    disconnect(p_hwm,&HardwareManager::readyForPoint,p_am,&AcquisitionManager::frequencyReady);

	if(aborted)
	    emit statusMessage(QString("Scan aborted"));
	else
	    emit statusMessage(QString("Scan complete"));

	configureUi(Idle);

}

void MainWindow::combUpdate(FreqCombData d)
{
	if(!d.success())
	{
		ui->repRateDoubleSpinBox->setValue(0.0);
		ui->combLockLed->setState(false);
		ui->oBeatDoubleSpinBox->setValue(0.0);
		ui->pBeatDoubleSpinBox->setValue(0.0);
		ui->sBeatDoubleSpinBox->setValue(0.0);
		ui->dnSpinBox->setValue(0);
		ui->combIdlerDoubleSpinBox->setValue(0.0);
	}
	else
	{
		ui->repRateDoubleSpinBox->setValue(d.repRate());
		ui->combLockLed->setState(d.repRateLocked());
		ui->oBeatDoubleSpinBox->setValue(d.offsetBeat());
		ui->pBeatDoubleSpinBox->setValue(d.pumpBeat());
		ui->sBeatDoubleSpinBox->setValue(d.signalBeat());
		ui->dnSpinBox->setValue(d.deltaN());
		ui->combIdlerDoubleSpinBox->setValue(d.calculatedIdlerFreq()/1e9);
	}

    ui->tsDateTimeEdit->setDateTime(QDateTime::currentDateTime());

	if(d_currentState == CombReading)
	{
        if(d.success())
            emit statusMessage(QString("Comb read complete"));
        else
            emit statusMessage(QString("Comb read aborted"));

		configureUi(Idle);
	}
}

void MainWindow::repRateUpdate(double r)
{
    ui->repRateDoubleSpinBox->setValue(r);
}

void MainWindow::startScan(Scan::ScanType t)
{
    if(p_batchThread->isRunning())
        return;

    QDialog d(this);
    d.setWindowTitle(QString("Configure Scan"));
    ScanConfigWidget *scw = new ScanConfigWidget(t,&d);
    QVBoxLayout *vbl = new QVBoxLayout();
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    vbl->addWidget(scw,1);
    vbl->addWidget(bb,0);
    d.setLayout(vbl);

    connect(bb->button(QDialogButtonBox::Cancel),&QPushButton::clicked,&d,&QDialog::reject);
    connect(bb->button(QDialogButtonBox::Ok),&QPushButton::clicked,scw,&ScanConfigWidget::validate);
    connect(scw,&ScanConfigWidget::scanValid,&d,&QDialog::accept);

    if(d.exec() == QDialog::Rejected)
        return;

    beginBatch(scw->toBatchManager());
}

void MainWindow::beginBatch(BatchManager *bm)
{
	connect(p_batchThread,&QThread::started,bm,&BatchManager::beginNextScan);
	connect(bm,&BatchManager::logMessage,p_lh,&LogHandler::logMessage);
    connect(bm,&BatchManager::beginScan,p_hwm,&HardwareManager::beginScanInitialization);
	connect(p_am,&AcquisitionManager::scanComplete,bm,&BatchManager::scanComplete);
	connect(bm,&BatchManager::batchComplete,this,&MainWindow::batchComplete);
	connect(bm,&BatchManager::batchComplete,p_batchThread,&QThread::quit);
	connect(p_batchThread,&QThread::finished,bm,&BatchManager::deleteLater);

    connect(p_hwm,&HardwareManager::readyForPoint,p_am,&AcquisitionManager::frequencyReady,Qt::UniqueConnection);
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

    //can iterate through hardware list here for more fine-grained control
    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    bool aom = set.value(QString("aomSynth/connected"),false).toBool();
    bool wavemeter = set.value(QString("wavemeter/connected"),false).toBool();
    bool comb = set.value(QString("freqComb/connected"),false).toBool();

	switch(s)
	{
	case Acquiring:
		ui->actionStart_Laser_Scan->setEnabled(false);
        ui->actionStart_Comb_Scan->setEnabled(false);
		ui->actionAbort->setEnabled(true);
		ui->actionCommunication->setEnabled(false);
		ui->actionTest_All_Connections->setEnabled(false);
		p_laserSlewAction->setEnabled(false);
		p_readCombAction->setEnabled(false);
		ui->actionIOBoard->setEnabled(false);
        ui->actionFrequency_Comb->setEnabled(false);
		break;
	case Slewing:
	case CombReading:
		ui->actionStart_Laser_Scan->setEnabled(false);
        ui->actionStart_Comb_Scan->setEnabled(false);
		ui->actionAbort->setEnabled(true);
		ui->actionCommunication->setEnabled(false);
		ui->actionTest_All_Connections->setEnabled(false);
		p_laserSlewAction->setEnabled(false);
		p_readCombAction->setEnabled(false);
		ui->actionIOBoard->setEnabled(false);
        ui->actionFrequency_Comb->setEnabled(false);
		break;
	case Disconnected:
		ui->actionStart_Laser_Scan->setEnabled(false);
        ui->actionStart_Comb_Scan->setEnabled(false);
		ui->actionAbort->setEnabled(false);
		ui->actionCommunication->setEnabled(true);
		ui->actionTest_All_Connections->setEnabled(true);
		p_laserSlewAction->setEnabled(false);
		p_readCombAction->setEnabled(false);
		ui->actionIOBoard->setEnabled(true);
        ui->actionFrequency_Comb->setEnabled(true);
		break;
	case Idle:
	default:
		ui->actionStart_Laser_Scan->setEnabled(true);
        ui->actionStart_Comb_Scan->setEnabled(aom && wavemeter && comb);
		ui->actionAbort->setEnabled(false);
		ui->actionCommunication->setEnabled(true);
		ui->actionTest_All_Connections->setEnabled(true);
		p_laserSlewAction->setEnabled(true);
		p_readCombAction->setEnabled(true);
		ui->actionIOBoard->setEnabled(true);
        ui->actionFrequency_Comb->setEnabled(true);
		break;
	}

	if(s != Disconnected)
		d_currentState = s;
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
	if(p_batchThread->isRunning())
	    ev->ignore();
	else
	    ev->accept();

}

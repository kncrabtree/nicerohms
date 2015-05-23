#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QLabel>

#include "loghandler.h"
#include "communicationdialog.h"
#include "hardwaremanager.h"

MainWindow::MainWindow(QWidget *parent) :
     QMainWindow(parent),
     ui(new Ui::MainWindow)
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

	QThread *hwmThread = new QThread(this);
	connect(hwmThread,&QThread::started,p_hwm,&HardwareManager::initialize);
	connect(hwmThread,&QThread::finished,p_hwm,&HardwareManager::deleteLater);
	p_hwm->moveToThread(hwmThread);
	d_threadObjectList.append(qMakePair(hwmThread,p_hwm));

	connect(ui->actionCommunication,&QAction::triggered,this,&MainWindow::launchCommunicationDialog);
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

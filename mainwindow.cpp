#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>

#include "loghandler.h"

MainWindow::MainWindow(QWidget *parent) :
     QMainWindow(parent),
     ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	p_lh = new LogHandler();
	connect(p_lh,&LogHandler::sendLogMessage,ui->logTextEdit,&QTextEdit::append);

	QThread *lhThread = new QThread(this);
	connect(lhThread,&QThread::finished,p_lh,&LogHandler::deleteLater);
	p_lh->moveToThread(lhThread);
	d_threadObjectList.append(qMakePair(lhThread,p_lh));
	lhThread->start();
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

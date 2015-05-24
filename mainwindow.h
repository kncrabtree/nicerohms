#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QList>
#include <QPair>

#include "scan.h"

class QThread;
class LogHandler;
class HardwareManager;
class AcquisitionManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void launchCommunicationDialog();

	void manualRelock();
	void scanInitialized(const Scan s);

signals:
    void statusMessage(const QString);
    void manualRelockComplete(bool abort);

private:
	Ui::MainWindow *ui;

	LogHandler *p_lh;
	HardwareManager *p_hwm;
	AcquisitionManager *p_am;

	QList<QPair<QThread*,QObject*>> d_threadObjectList;
};

#endif // MAINWINDOW_H

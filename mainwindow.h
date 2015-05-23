#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QList>
#include <QPair>

class QThread;
class LogHandler;
class HardwareManager;

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

signals:
    void statusMessage(const QString);

private:
	Ui::MainWindow *ui;

	LogHandler *p_lh;
	HardwareManager *p_hwm;

	QList<QPair<QThread*,QObject*>> d_threadObjectList;
};

#endif // MAINWINDOW_H

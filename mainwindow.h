#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QList>
#include <QPair>

class QThread;
class LogHandler;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow *ui;

	LogHandler *p_lh;

	QList<QPair<QThread*,QObject*>> d_threadObjectList;
};

#endif // MAINWINDOW_H

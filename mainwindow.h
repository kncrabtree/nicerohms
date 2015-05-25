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
class BatchManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	enum UiState {
		Idle,
		Acquiring,
		Slewing,
		Disconnected
	};

public slots:
	void launchCommunicationDialog();
	void hardwareConnected(bool connected);
	void configForSlew(bool slewing);

	void manualRelock();
	void scanInitialized(const Scan s);
	void batchComplete(bool aborted);

	void test();
	void setWavemeterUnits(NicerOhms::WmDisplayUnits u);
	void readWavemeterSettings();
	void pumpUpdate(double f);
	void signalUpdate(double f);


signals:
    void statusMessage(const QString);
    void manualRelockComplete(bool abort);

private:
	Ui::MainWindow *ui;
	UiState d_currentState;
	NicerOhms::WmDisplayUnits d_wmUnits;
	bool d_hardwareConnected;

	LogHandler *p_lh;
	HardwareManager *p_hwm;
	AcquisitionManager *p_am;

	QThread *p_batchThread;

	QList<QPair<QThread*,QObject*>> d_threadObjectList;

	void beginBatch(BatchManager *bm);
	void configureUi(UiState s);

protected:
	void closeEvent(QCloseEvent *ev);
};



#endif // MAINWINDOW_H

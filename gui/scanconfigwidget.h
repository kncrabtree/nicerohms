#ifndef SCANCONFIGWIDGET_H
#define SCANCONFIGWIDGET_H

#include <QWidget>

#include <data/scan.h>

class BatchManager;

namespace Ui {
class ScanConfigWidget;
}

class ScanConfigWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScanConfigWidget(Scan::ScanType t, QWidget *parent = 0);
	~ScanConfigWidget();

	BatchManager *toBatchManager();

signals:
	void scanValid();
    void slewLaser(double voltage);
    void setPumpSign(bool);
    void setSignalSign(bool);
    void stepOnce();

public slots:
	void swapLaserStartStop();
	void validate();

private slots:
    void on_SignalRadioButton_clicked();

    void on_pumpRadioButton_clicked();

    void on_UpPushButton_clicked();

    void on_DownPushButton_clicked();

    void on_PumpToolButton_clicked(bool checked);

    void on_PumpToolButton_toggled(bool checked);

    void on_SignalToolButton_toggled(bool checked);

    void on_SignalToolButton_clicked(bool checked);

    void on_pumpLockRadioButton_toggled(bool checked);

    void on_pumpRadioButton_toggled(bool checked);

    void on_PumpToolButton_clicked();

private:
	Ui::ScanConfigWidget *ui;

	Scan::ScanType d_type;
	const QString d_key = QString("lastScanConfig");

	void saveToSettings() const;
};

#endif // SCANCONFIGWIDGET_H

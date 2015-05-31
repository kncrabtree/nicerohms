#ifndef SCANCONFIGWIDGET_H
#define SCANCONFIGWIDGET_H

#include <QWidget>

#include "scan.h"

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

public slots:
	void swapLaserStartStop();
	void validate();

private:
	Ui::ScanConfigWidget *ui;

	Scan::ScanType d_type;
	const QString d_key = QString("lastScanConfig");

	void saveToSettings() const;
};

#endif // SCANCONFIGWIDGET_H

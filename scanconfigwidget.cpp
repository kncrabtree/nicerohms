#include "scanconfigwidget.h"
#include "ui_scanconfigwidget.h"

#include "scan.h"
#include "batchsingle.h"
#include "ioboardanalogconfigmodel.h"

#include <QSettings>
#include <QApplication>
#include <QMessageBox>

ScanConfigWidget::ScanConfigWidget(Scan::ScanType t, QWidget *parent) :
     QWidget(parent),
	ui(new Ui::ScanConfigWidget), d_type(t)
{
	ui->setupUi(this);


	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	QString sk;
	if(t == Scan::LaserScan)
	{
		ui->combScanBox->hide();

		sk = s.value(QString("laser/subKey"),QString("virtual")).toString();
		QString units = s.value(QString("laser/%1/units").arg(sk),QString("V")).toString();
		if(!units.startsWith(QChar(' ')))
			units.prepend(' ');
		double min = s.value(QString("laser/%1/minPos").arg(sk),0.0).toDouble();
		double max = s.value(QString("laser/%1/maxPos").arg(sk),150.0).toDouble();
		double step = s.value(QString("laser/%1/controlStep").arg(sk),1.0).toDouble();
		int dec = s.value(QString("laser/%1/decimals").arg(sk),3).toInt();

		ui->laserStartDoubleSpinBox->setRange(min,max);
		ui->laserStartDoubleSpinBox->setSuffix(units);
		ui->laserStartDoubleSpinBox->setDecimals(dec);
		ui->laserStartDoubleSpinBox->setSingleStep(step);

		ui->laserStopDoubleSpinBox->setRange(min,max);
		ui->laserStopDoubleSpinBox->setSuffix(units);
		ui->laserStopDoubleSpinBox->setDecimals(dec);
		ui->laserStopDoubleSpinBox->setSingleStep(step);

		double minStep = pow(10.0,static_cast<double>(-dec));
		ui->laserStepDoubleSpinBox->setRange(minStep,max);
		ui->laserStepDoubleSpinBox->setSuffix(units);
		ui->laserStepDoubleSpinBox->setDecimals(dec);
		ui->laserStepDoubleSpinBox->setSingleStep(minStep*10.0);

		double lastStart = s.value(QString("%1/laserStart").arg(d_key),10.0).toDouble();
		double lastStop = s.value(QString("%1/laserStop").arg(d_key),20.0).toDouble();
		double lastStep = s.value(QString("%1/laserStep").arg(d_key),0.1).toDouble();
		int lastDelay = s.value(QString("%1/laserDelay").arg(d_key),200).toInt();

		ui->laserStartDoubleSpinBox->setValue(lastStart);
		ui->laserStopDoubleSpinBox->setValue(lastStop);
		ui->laserStepDoubleSpinBox->setValue(lastStep);
		ui->laserDelaySpinBox->setValue(lastDelay);

		bool combAlive = s.value(QString("freqComb/connected"),false).toBool();
		bool comb = s.value(QString("%1/combEnabled").arg(d_key),false).toBool();
		if(!combAlive)
		{
			ui->freqCombCheckBox->setChecked(false);
			ui->freqCombCheckBox->setEnabled(false);
		}
		else
			ui->freqCombCheckBox->setChecked(comb);

		bool wmAlive = s.value(QString("wavemeter/connected"),false).toBool();
		bool wm = s.value(QString("%1/wavemeterEnabled").arg(d_key),true).toBool();
		if(!wmAlive)
		{
			ui->wavemeterCheckBox->setChecked(false);
			ui->wavemeterCheckBox->setEnabled(false);
		}
		else
			ui->wavemeterCheckBox->setChecked(wm);

		bool aomAlive = s.value(QString("aomSynth/connected"),false).toBool();
		bool aom = s.value(QString("%1/aomEnabled").arg(d_key),false).toBool();
		if(!aomAlive)
		{
			ui->aomCheckBox->setChecked(false);
			ui->aomCheckBox->setEnabled(false);
		}
		else
			ui->aomCheckBox->setChecked(aom);

		connect(ui->swapButton,&QPushButton::clicked,this,&ScanConfigWidget::swapLaserStartStop);
	}
	else
	{
		ui->laserScanBox->hide();

		ui->freqCombCheckBox->setChecked(true);
		ui->freqCombCheckBox->setEnabled(false);

		ui->wavemeterCheckBox->setChecked(true);
		ui->wavemeterCheckBox->setEnabled(false);

		ui->aomCheckBox->setChecked(true);
		ui->aomCheckBox->setEnabled(false);

		double length = s.value(QString("%1/combLength").arg(d_key),150.0).toDouble();
		double step = s.value(QString("%1/combStep").arg(d_key),2.0).toDouble();
		int lastDelay = s.value(QString("%1/combDelay").arg(d_key),1000).toInt();

		ui->combLengthDoubleSpinBox->setValue(length);
		ui->combStepDoubleSpinBox->setValue(step);
		ui->combDelaySpinBox->setValue(lastDelay);
	}

	bool l1Alive = s.value(QString("lockin1/connected"),false).toBool();
	bool l1 = s.value(QString("%1/lockin1Enabled").arg(d_key),false).toBool();
	if(!l1Alive)
	{
		ui->lockin1CheckBox->setChecked(false);
		ui->lockin1CheckBox->setEnabled(false);
	}
	else
		ui->lockin1CheckBox->setChecked(l1);

	bool l2Alive = s.value(QString("lockin2/connected"),false).toBool();
	bool l2 = s.value(QString("%1/lockin2Enabled").arg(d_key),false).toBool();
	if(!l2Alive)
	{
		ui->lockin2CheckBox->setChecked(false);
		ui->lockin2CheckBox->setEnabled(false);
	}
	else
		ui->lockin2CheckBox->setChecked(l2);

	bool cpztAlive = s.value(QString("cavityPZT/connected"),false).toBool();
	if(!cpztAlive)
		ui->cavityPZTBox->setChecked(false);
	else
	{
		double low = s.value(QString("%1/lowTrip").arg(d_key),10.0).toDouble();
		double high = s.value(QString("%1/highTrip").arg(d_key),140.0).toDouble();

		ui->lowTripDoubleSpinBox->setValue(low);
		ui->highTripDoubleSpinBox->setValue(high);

		bool ar = s.value(QString("%1/autoRelock").arg(d_key),true).toBool();
		bool af = s.value(QString("%1/abortOnFail").arg(d_key),false).toBool();

		ui->autoRelockCheckBox->setChecked(ar);
		ui->abortOnFailCheckBox->setChecked(af);
	}

	ui->commentsBox->setPlainText(s.value(QString("%1/comments").arg(d_key),QString("")).toString());

	//initialize models
	ui->ioBoardTableView->setModel(new IOBoardAnalogConfigModel(this));
	ui->ioBoardTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->ioBoardTableView->setItemDelegateForColumn(2,new RangeDelegate);
}

ScanConfigWidget::~ScanConfigWidget()
{
	delete ui;
}

BatchManager *ScanConfigWidget::toBatchManager()
{
	Scan s(d_type);

	s.addHardwareItem(QString("laser"));
	if(d_type == Scan::LaserScan)
	{
		s.setScanParams(ui->laserStartDoubleSpinBox->value(),ui->laserStopDoubleSpinBox->value(),
					 ui->laserStepDoubleSpinBox->value(),ui->laserDelaySpinBox->value());

		s.addHardwareItem(QString("wavemeter"),ui->wavemeterCheckBox->isChecked());
		s.addHardwareItem(QString("aomSynth"),ui->aomCheckBox->isChecked());
		s.addHardwareItem(QString("freqComb"),ui->freqCombCheckBox->isChecked());
	}
	else
	{
		s.setScanParams(0.0,ui->combLengthDoubleSpinBox->value(),ui->combStepDoubleSpinBox->value(),ui->combDelaySpinBox->value());

		s.addHardwareItem(QString("wavemeter"),true);
		s.addHardwareItem(QString("aomSynth"),true);
		s.addHardwareItem(QString("freqComb"),true);
	}

	s.addHardwareItem(QString("cavityPZT"),ui->cavityPZTBox->isChecked());
	if(ui->cavityPZTBox->isChecked())
	{
		s.setCavityPZTRange(qMin(ui->lowTripDoubleSpinBox->value(),ui->highTripDoubleSpinBox->value()),
						qMax(ui->lowTripDoubleSpinBox->value(),ui->highTripDoubleSpinBox->value()));
		s.setAutoRelock(ui->autoRelockCheckBox->isChecked());
		s.setAbortOnUnlock(ui->abortOnFailCheckBox->isChecked());
	}
	else
	{
		s.setAutoRelock(false);
		s.setAbortOnUnlock(false);
		s.setCavityPZTRange(-1.0,1.0);
	}

	s.addHardwareItem(QString("lockin1"),ui->lockin1CheckBox->isChecked());
	s.addHardwareItem(QString("lockin2"),ui->lockin2CheckBox->isChecked());

	s.setComments(ui->commentsBox->toPlainText());

	return new BatchSingle(s);
}

void ScanConfigWidget::swapLaserStartStop()
{
	double start = ui->laserStartDoubleSpinBox->value();
	double stop = ui->laserStopDoubleSpinBox->value();

	qSwap(start,stop);

	ui->laserStartDoubleSpinBox->setValue(start);
	ui->laserStopDoubleSpinBox->setValue(stop);
}

void ScanConfigWidget::validate()
{
	if(d_type == Scan::LaserScan)
	{
		if(fabs(ui->laserStartDoubleSpinBox->value() - ui->laserStopDoubleSpinBox->value()) > ui->laserStepDoubleSpinBox->value())
		{
			saveToSettings();
			emit scanValid();
		}
		else
			QMessageBox::warning(this,QString("Invalid Scan Settings"),QString("The scan could not be created because the scan length is less than the step size."));
	}
	else
	{
		if(fabs(ui->combLengthDoubleSpinBox->value()) > fabs(ui->combStepDoubleSpinBox->value()))
		{
			saveToSettings();
			emit scanValid();
		}
		else
			QMessageBox::warning(this,QString("Invalid Scan Settings"),QString("The scan could not be created because the scan length is less than the step size."));
	}
}

void ScanConfigWidget::saveToSettings() const
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(d_key);

	if(d_type == Scan::LaserScan)
	{
		s.setValue(QString("laserStart"),ui->laserStartDoubleSpinBox->value());
		s.setValue(QString("laserStop"),ui->laserStopDoubleSpinBox->value());
		s.setValue(QString("laserStep"),ui->laserStepDoubleSpinBox->value());
		s.setValue(QString("laserDelay"),ui->laserDelaySpinBox->value());

		if(ui->wavemeterCheckBox->isEnabled())
			s.setValue(QString("wavemeterEnabled"),ui->wavemeterCheckBox->isChecked());
		if(ui->aomCheckBox->isEnabled())
			s.setValue(QString("aomEnabled"),ui->aomCheckBox->isChecked());
		if(ui->freqCombCheckBox->isEnabled())
			s.setValue(QString("freqCombEnabled"),ui->freqCombCheckBox->isChecked());
	}
	else
	{
		s.setValue(QString("combLength"),ui->combLengthDoubleSpinBox->value());
		s.setValue(QString("combStep"),ui->combStepDoubleSpinBox->value());
		s.setValue(QString("combDelay"),ui->combDelaySpinBox->value());
	}

	if(ui->cavityPZTBox->isChecked())
	{
		s.setValue(QString("lowTrip"),ui->lowTripDoubleSpinBox->value());
		s.setValue(QString("highTrip"),ui->highTripDoubleSpinBox->value());
		s.setValue(QString("autoRelock"),ui->autoRelockCheckBox->isChecked());
		s.setValue(QString("abortOnFail"),ui->abortOnFailCheckBox->isChecked());
	}

	if(ui->lockin1CheckBox->isEnabled())
		s.setValue(QString("lockin1Enabled"),ui->lockin1CheckBox->isChecked());
	if(ui->lockin2CheckBox->isEnabled())
		s.setValue(QString("lockin2Enabled"),ui->lockin2CheckBox->isChecked());

	s.setValue(QString("comments"),ui->commentsBox->toPlainText());


	//ioboard and validation
	static_cast<IOBoardAnalogConfigModel*>(ui->ioBoardTableView->model())->saveToSettings();

	s.endGroup();
	s.sync();
}

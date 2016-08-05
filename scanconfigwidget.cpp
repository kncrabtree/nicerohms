#include "scanconfigwidget.h"
#include "ui_scanconfigwidget.h"

#include "scan.h"
#include "batchsingle.h"
#include "ioboardanalogconfigmodel.h"
#include "ioboarddigitalconfigmodel.h"
#include "validationmodel.h"

#include <QSettings>
#include <QApplication>
#include <QMessageBox>

#include <QDebug>
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

        bool cb = s.value(QString("%1/cavityPZT").arg(d_key),false).toBool();
        ui->cavityPZTBox->setChecked(cb);


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
        connect(this,&ScanConfigWidget::stepOnce,this,&ScanConfigWidget::on_UpPushButton_clicked);
		ui->laserScanBox->hide();

		ui->wavemeterCheckBox->setChecked(true);
		ui->wavemeterCheckBox->setEnabled(false);

        ui->SignalRadioButton->setChecked(true);
        ui->pumpLockCheckBox->setChecked(true);

        ui->aomCheckBox->setChecked(false);
        ui->aomCheckBox->setEnabled(false);


        emit setPumpSign(true);
        emit setSignalSign(true);
        emit stepOnce();




		double length = s.value(QString("%1/combLength").arg(d_key),150.0).toDouble();
		double step = s.value(QString("%1/combStep").arg(d_key),2.0).toDouble();
		int lastDelay = s.value(QString("%1/combDelay").arg(d_key),1000).toInt();

        slewLaser(s.value(QString("%1/laserStart").arg(d_key),0).toDouble());

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

        bool ar = s.value(QString("%1/autoRelock").arg(d_key),false).toBool();
		bool af = s.value(QString("%1/abortOnFail").arg(d_key),false).toBool();

		ui->autoRelockCheckBox->setChecked(ar);
		ui->abortOnFailCheckBox->setChecked(af);
	}

	ui->commentsBox->setPlainText(s.value(QString("%1/comments").arg(d_key),QString("")).toString());

	//initialize models
	ui->ioBoardTableView->setModel(new IOBoardAnalogConfigModel(this));
	ui->ioBoardTableView->setItemDelegateForColumn(1,new RangeDelegate);
	ui->ioBoardTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	ui->ioBoardDigitalTableView->setModel(new IOBoardDigitalConfigModel(this));
	ui->ioBoardDigitalTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	ValidationModel *vm = new ValidationModel(this);
	ui->validationTableView->setModel(vm);
	ui->validationTableView->setItemDelegateForColumn(0,new CompleterLineEditDelegate);
	ui->validationTableView->setItemDelegateForColumn(1,new DoubleSpinBoxDelegate);
	ui->validationTableView->setItemDelegateForColumn(2,new DoubleSpinBoxDelegate);
	ui->validationTableView->setItemDelegateForColumn(3,new ActionComboBoxDelegate);
	ui->validationTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	connect(ui->addValidationButton,&QToolButton::clicked,[=](){ vm->addNewItem(); });
	connect(ui->removeValidationButton,&QToolButton::clicked,[=](){
		QModelIndexList l = ui->validationTableView->selectionModel()->selectedIndexes();
		if(l.isEmpty())
			return;

		QList<int> rowList;
		for(int i=0; i<l.size(); i++)
		{
			if(!rowList.contains(l.at(i).row()))
				rowList.append(l.at(i).row());
		}

		std::stable_sort(rowList.begin(),rowList.end());

		for(int i=rowList.size()-1; i>=0; i--)
			vm->removeRows(rowList.at(i),1,QModelIndex());
	});
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
	}
	else
	{
		s.setScanParams(0.0,ui->combLengthDoubleSpinBox->value(),ui->combStepDoubleSpinBox->value(),ui->combDelaySpinBox->value());

		s.addHardwareItem(QString("wavemeter"),true);
        s.addHardwareItem(QString("aomSynth"),ui->aomCheckBox->isChecked());
		s.addHardwareItem(QString("freqComb"),true);
        s.addHardwareItem(QString("frequencyCounter"),ui->pumpLockCheckBox->isChecked());
            if(ui->pumpLockCheckBox->isChecked())
            {
                s.setPumpLock(ui->pumpLockCheckBox->isChecked());

            }
            emit setPumpSign(!ui->PumpToolButton->isChecked());
            emit setSignalSign(!ui->SignalToolButton->isChecked());
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

	s.addHardwareItem(QString("ioboard"));
	//add ioboard settngs
	s.setIOBoardAnalog(static_cast<IOBoardAnalogConfigModel*>(ui->ioBoardTableView->model())->getConfig());
	s.setIOBoardDigital(static_cast<IOBoardDigitalConfigModel*>(ui->ioBoardDigitalTableView->model())->getConfig());

	//add validation settings
	QList<NicerOhms::ValidationItem> l = static_cast<ValidationModel*>(ui->validationTableView->model())->getList();
	for(int i=0; i<l.size(); i++)
	{
		if(!l.at(i).key.isEmpty())
		{
			Scan::PointAction a;
			if(l.at(i).abort)
				a = Scan::Abort;
			else
				a = Scan::Remeasure;

			s.addValidationItem(l.at(i).key,qMin(l.at(i).min,l.at(i).max),qMin(l.at(i).min,l.at(i).max),a);
		}
	}

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
	}
	else
	{
		s.setValue(QString("combLength"),ui->combLengthDoubleSpinBox->value());
		s.setValue(QString("combStep"),ui->combStepDoubleSpinBox->value());
		s.setValue(QString("combDelay"),ui->combDelaySpinBox->value());
        s.setValue(QString("signalLock"),ui->SignalRadioButton->isChecked());
        s.setValue(QString("pumpLock"),ui->pumpLockCheckBox->isChecked());


	}

    s.setValue(QString("cavityPZT"),ui->cavityPZTBox->isChecked());

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
	static_cast<IOBoardDigitalConfigModel*>(ui->ioBoardDigitalTableView->model())->saveToSettings();

	static_cast<ValidationModel*>(ui->validationTableView->model())->saveToSettings();

	s.endGroup();
	s.sync();
}

void ScanConfigWidget::on_SignalRadioButton_clicked()
{
    ui->UpPushButton->setEnabled(true);
    ui->DownPushButton->setEnabled(true);
}

void ScanConfigWidget::on_pumpRadioButton_clicked()
{
    ui->UpPushButton->setEnabled(false);
    ui->DownPushButton->setEnabled(false);
}

void ScanConfigWidget::on_UpPushButton_clicked()
{
    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
     set.beginGroup("lastScanConfig");


     double voltage = set.value("laserStart",20).toDouble() + ui->combStepDoubleSpinBox->value()*set.value("MHzToV",.003).toDouble();

     emit slewLaser(voltage);

     set.setValue(QString("laserStart"),voltage);

     set.endGroup();

}

void ScanConfigWidget::on_DownPushButton_clicked()
{
    QSettings set(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
     set.beginGroup("lastScanConfig");


     double voltage = set.value("laserStart",20).toDouble() - ui->combStepDoubleSpinBox->value()*set.value("MHzToV",.003).toDouble();

     emit slewLaser(voltage);


     set.setValue(QString("laserStart"),voltage);

     set.endGroup();

}


void ScanConfigWidget::on_PumpToolButton_toggled(bool checked)
{
    if(!checked)
    {
        ui->PumpToolButton->setText("+");
    }
    else
        ui->PumpToolButton->setText(QString::fromUtf16(u"−"));

}

void ScanConfigWidget::on_SignalToolButton_toggled(bool checked)
{
    if(!checked)
    {
        ui->SignalToolButton->setText("+");
    }
    else
        ui->SignalToolButton->setText(QString::fromUtf16(u"−"));

}

void ScanConfigWidget::on_SignalToolButton_clicked(bool checked)
{
    emit setSignalSign(!checked);

}

void ScanConfigWidget::on_PumpToolButton_clicked(bool checked)
{
    emit setPumpSign(!checked);


}

void ScanConfigWidget::on_pumpLockCheckBox_toggled(bool checked)
{
    ui->aomCheckBox->setChecked(!checked);
}

void ScanConfigWidget::on_pumpRadioButton_toggled(bool checked)
{
    ui->pumpLockCheckBox->setChecked(false);
}

void ScanConfigWidget::on_PumpToolButton_clicked()
{

}

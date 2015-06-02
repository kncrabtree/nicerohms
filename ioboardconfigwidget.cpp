#include "ioboardconfigwidget.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSettings>
#include <QApplication>
#include <QMessageBox>

IOBoardConfigWidget::IOBoardConfigWidget(QWidget *parent) : QWidget(parent)
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	QString sk = s.value(QString("ioboard/subKey"),QString("virtual")).toString();
	s.beginGroup(QString("ioboard"));
	s.beginGroup(sk);

	QFormLayout *fl = new QFormLayout;

	p_serialNoEdit = new QLineEdit(this);
	p_serialNoEdit->setText(s.value(QString("serialNo"),QString("0")).toString());
	fl->addRow(QString("Serial No"),p_serialNoEdit);

	p_testButton = new QPushButton(QString("Test Connection"),this);
	connect(p_testButton,&QPushButton::clicked,this,&IOBoardConfigWidget::test);
	fl->addRow(QString(""),p_testButton);

	p_readLockIntervalSpinBox = new QSpinBox(this);
	p_readLockIntervalSpinBox->setRange(0,10000);
	p_readLockIntervalSpinBox->setValue(s.value(QString("readLockInterval"),5).toInt());
	p_readLockIntervalSpinBox->setSingleStep(5);
	p_readLockIntervalSpinBox->setSuffix(QString(" ms"));
	fl->addRow(QString("Lock Check Interval"),p_readLockIntervalSpinBox);

	p_relockStepSpinBox = new QDoubleSpinBox(this);
	p_relockStepSpinBox->setDecimals(1);
	p_relockStepSpinBox->setSuffix(QString(" mV"));
	p_relockStepSpinBox->setRange(0.1,1000.0);
	p_relockStepSpinBox->setValue(s.value(QString("relockStep"),0.1).toDouble()*1000.0);
	fl->addRow(QString("Relock Step Size"),p_relockStepSpinBox);

	s.endGroup();
	s.endGroup();

	setLayout(fl);
}

IOBoardConfigWidget::~IOBoardConfigWidget()
{

}

void IOBoardConfigWidget::test()
{
	setEnabled(false);
	setCursor(Qt::BusyCursor);

	saveToSettings();

	emit testConnection(QString(""),QString("ioboard"));
}

void IOBoardConfigWidget::testComplete(QString device, bool success, QString msg)
{
	setEnabled(true);
	setCursor(QCursor());

	if(success)
		QMessageBox::information(this,QString("Connection Successful"),
							QString("%1 connected successfully!").arg(device),QMessageBox::Ok);
	else
		QMessageBox::critical(this,QString("Connection failed"),
						  QString("%1 connection failed!\n%2").arg(device).arg(msg),QMessageBox::Ok);
}

void IOBoardConfigWidget::saveToSettings()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	QString sk = s.value(QString("ioboard/subKey"),QString("virtual")).toString();
	s.beginGroup(QString("ioboard"));
	s.beginGroup(sk);

	s.setValue(QString("serialNo"),p_serialNoEdit->text());
	s.setValue(QString("readLockInterval"),p_readLockIntervalSpinBox->value());
	s.setValue(QString("relockStep"),p_relockStepSpinBox->value()/1000.0);
	s.endGroup();
	s.endGroup();
	s.sync();
}


#include "freqcombconfigwidget.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QSettings>
#include <QApplication>
#include <QMessageBox>

FreqCombConfigWidget::FreqCombConfigWidget(QWidget *parent) : QWidget(parent)
{
    QFormLayout *fl = new QFormLayout;
    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    QString sk = s.value(QString("freqComb/subKey"),QString("virtual")).toString();
    s.beginGroup(QString("freqComb"));
    s.beginGroup(sk);

    p_uriEdit = new QLineEdit(this);
    p_uriEdit->setText(s.value(QString("uri"),QString("")).toString());

    fl->addRow(QString("URI"),p_uriEdit);

    QPushButton *testButton = new QPushButton(QString("Test Connection"),this);
    connect(testButton,&QPushButton::clicked,this,&FreqCombConfigWidget::test);
    fl->addRow(QString(""),testButton);

    p_wmReadsBox = new QSpinBox(this);
    p_wmReadsBox->setRange(1,1000);
    p_wmReadsBox->setValue(s.value(QString("wavemeterReads"),10).toInt());
    p_wmReadsBox->setToolTip(QString("Number of wavemeter readings to average to determine comb mode numbers."));
    fl->addRow(QString("Wavemeter reads"),p_wmReadsBox);

    p_counter1RadioButton = new QRadioButton(this);
    p_counter2RadioButton = new QRadioButton(this);

    p_counter1RadioButton->setChecked(s.value("counterConfig").toBool());
    p_counter1RadioButton->setText("Pump Counter 1, Signal Counter 2");
    p_counter1RadioButton->setToolTip("Counter 1 has internal bandpass, counter 2 has no internal filter");
    p_counter2RadioButton->setChecked(!s.value("counterConfig").toBool());
    p_counter2RadioButton->setText("Signal Counter 1, Pump Counter 2");

    fl->addRow(QString("Configuration 1"),p_counter1RadioButton);
    fl->addRow(QString("Configuration 2"),p_counter2RadioButton);



    fl->setLabelAlignment(Qt::AlignRight);
    s.endGroup();
    s.endGroup();

    setLayout(fl);
}

FreqCombConfigWidget::~FreqCombConfigWidget()
{

}

void FreqCombConfigWidget::test()
{
    setEnabled(false);
    setCursor(Qt::BusyCursor);

    saveToSettings();

    emit testConnection(QString(""),QString("freqComb"));
}

void FreqCombConfigWidget::testComplete(QString device, bool success, QString msg)
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

void FreqCombConfigWidget::saveToSettings()
{
    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    QString sk = s.value(QString("freqComb/subKey"),QString("virtual")).toString();
    s.beginGroup(QString("freqComb"));
    s.beginGroup(sk);

    s.setValue(QString("uri"),p_uriEdit->text());
    s.setValue(QString("wavemeterReads"),p_wmReadsBox->value());
    s.setValue(QString("counterConfig"),p_counter1RadioButton->isChecked());

    s.endGroup();
    s.endGroup();
    s.sync();
}


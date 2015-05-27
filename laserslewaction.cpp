#include "laserslewaction.h"

#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSettings>
#include <QApplication>

LaserSlewAction::LaserSlewAction(QWidget *parent) :
    QWidgetAction(parent)
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *hbl = new QHBoxLayout;

    p_label = new QLabel(QString("Laser Position:"),parent);
    p_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    hbl->addWidget(p_label,0,Qt::AlignRight|Qt::AlignVCenter);

    p_posBox = new QDoubleSpinBox(parent);
    hbl->addWidget(p_posBox,0,Qt::AlignCenter);

    p_setButton = new QPushButton(QString("Set"),parent);
    connect(p_setButton,&QPushButton::clicked,this,&QWidgetAction::trigger);
    hbl->addWidget(p_setButton,0,Qt::AlignLeft|Qt::AlignVCenter);

    w->setLayout(hbl);
    setDefaultWidget(w);
    readLaserConfig();

    connect(this,&LaserSlewAction::triggered,[=](){ emit slewSignal(targetLaserPosition()); });
}

LaserSlewAction::~LaserSlewAction()
{

}

double LaserSlewAction::targetLaserPosition()
{
    return p_posBox->value();
}

void LaserSlewAction::readLaserConfig()
{
    QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
    s.beginGroup(QString("laser"));
    QString subKey = s.value(QString("subKey"),QString("virtual")).toString();
    double min = s.value(QString("%1/minPos").arg(subKey),0.0).toDouble();
    double max = s.value(QString("%1/maxPos").arg(subKey),100.0).toDouble();
    double step = fabs(s.value(QString("%1/controlStep").arg(subKey),1.0).toDouble());
    if(qFuzzyCompare(1.0+step,1.0))
        step = 1.0;
    int decimals = s.value(QString("%1/decimals").arg(subKey),3).toInt();
    QString units = s.value(QString("%1/%2/units").arg(subKey),QString(" V")).toString();
    if(!units.startsWith(QChar(' ')))
        units.prepend(QChar(' '));
    s.endGroup();

    p_posBox->setDecimals(decimals);
    p_posBox->setRange(min,max);
    p_posBox->setSuffix(units);
    p_posBox->setSingleStep(step);

    defaultWidget()->update();
}


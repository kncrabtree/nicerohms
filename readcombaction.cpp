#include "readcombaction.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include <QToolButton>
#include <QMenu>

ReadCombAction::ReadCombAction(QWidget *parent) :
	QWidgetAction(parent), d_pumpChecked(true), d_signalChecked(true)
{
}

ReadCombAction::~ReadCombAction()
{

}



QWidget *ReadCombAction::createWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *hbl = new QHBoxLayout;

	QToolButton *readButton = new QToolButton(parent);
	readButton->setText(QString("Read Comb"));
	readButton->setIcon(QIcon(QString(":/icons/comb.png")));
	connect(readButton,&QToolButton::clicked,this,&QWidgetAction::trigger);
	readButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	hbl->addWidget(readButton);

	QToolButton *cfgButton = new QToolButton(parent);
	cfgButton->setText(QString(""));
	cfgButton->setIcon(QIcon(QString(":/icons/configure.png")));
	cfgButton->setPopupMode(QToolButton::InstantPopup);

	QMenu *cfgMenu = new QMenu(parent);

	QWidgetAction *cfgwa = new QWidgetAction(cfgMenu);
	QWidget *cfgw = new QWidget(parent);
	QFormLayout *fl = new QFormLayout;
/*
	QToolButton *pButton = new QToolButton(cfgw);
	pButton->setText(QString("+"));
	pButton->setCheckable(true);
	pButton->setChecked(true);
	connect(pButton,&QToolButton::toggled,[=](bool ch){
		if(ch)
			pButton->setText(QString("+"));
		else
			pButton->setText(QString::fromUtf16(u"−"));
	});
	connect(pButton,&QToolButton::toggled,this,&ReadCombAction::setPumpSign);

	QToolButton *sButton = new QToolButton(cfgw);
	sButton->setText(QString("+"));
	sButton->setCheckable(true);
	sButton->setChecked(true);
	connect(sButton,&QToolButton::toggled,[=](bool ch){
		if(ch)
			sButton->setText(QString("+"));
		else
			sButton->setText(QString::fromUtf16(u"−"));
	});
	connect(sButton,&QToolButton::toggled,this,&ReadCombAction::setSignalSign);

	fl->addRow(QString("Pump Beat Sign"),pButton);
	fl->addRow(QString("Signal Beat Sign"),sButton);
	cfgw->setLayout(fl);
    */
	cfgwa->setDefaultWidget(cfgw);

	cfgMenu->addAction(cfgwa);
	cfgButton->setMenu(cfgMenu);

	hbl->addWidget(cfgButton);

	w->setLayout(hbl);
//	setDefaultWidget(w);

	return w;
}

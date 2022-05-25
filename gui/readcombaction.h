#ifndef READCOMBACTION_H
#define READCOMBACTION_H

#include <QWidgetAction>

class ReadCombAction : public QWidgetAction
{
	Q_OBJECT
public:
	ReadCombAction(QWidget *parent = nullptr);
	~ReadCombAction();

signals:
	void setPumpSign(bool);
	void setSignalSign(bool);

private:
	bool d_pumpChecked, d_signalChecked;

	// QWidgetAction interface
protected:
	QWidget *createWidget(QWidget *parent);
};

#endif // READCOMBACTION_H

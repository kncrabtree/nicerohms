#ifndef IOBOARDCONFIGWIDGET_H
#define IOBOARDCONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;

class IOBoardConfigWidget : public QWidget
{
	Q_OBJECT
public:
	explicit IOBoardConfigWidget(QWidget *parent = 0);
	~IOBoardConfigWidget();

signals:
	void testConnection(QString, QString);

public slots:
	void test();
	void testComplete(QString device, bool success, QString msg);
	void saveToSettings();

private:
	QLineEdit *p_serialNoEdit;
	QSpinBox *p_readLockIntervalSpinBox;
	QDoubleSpinBox *p_relockStepSpinBox;
	QPushButton *p_testButton;

};

#endif // IOBOARDCONFIGWIDGET_H

#ifndef FREQCOMBCONFIGWIDGET_H
#define FREQCOMBCONFIGWIDGET_H

#include <QWidget>

class QLineEdit;

class FreqCombConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FreqCombConfigWidget(QWidget *parent = 0);
    ~FreqCombConfigWidget();

signals:
    void testConnection(QString, QString);

public slots:
    void test();
    void testComplete(QString device, bool success, QString msg);
    void saveToSettings();

private:
    QLineEdit *p_uriEdit;
};

#endif // FREQCOMBCONFIGWIDGET_H

#ifndef LASERSLEWACTION_H
#define LASERSLEWACTION_H

#include <QWidgetAction>

class QLabel;
class QDoubleSpinBox;
class QPushButton;

class LaserSlewAction : public QWidgetAction
{
    Q_OBJECT
public:
    LaserSlewAction(QWidget *parent = nullptr);
    ~LaserSlewAction();

    double targetLaserPosition();

public slots:
    void readLaserConfig();

signals:
    void slewSignal(double);

private:
    QLabel *p_label;
    QDoubleSpinBox *p_posBox;
    QPushButton *p_setButton;
};

#endif // LASERSLEWACTION_H

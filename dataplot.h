#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "zoompanplot.h"

class QwtPlotCurve;


class DataPlot : public ZoomPanPlot
{
    Q_OBJECT

public:
    DataPlot(QString name, QWidget *parent = 0);
    ~DataPlot();

    void initializeLabel(QwtPlotCurve* curve, bool isVisible);

signals:
    void curveVisiblityToggled(QwtPlotCurve*,bool);
    void legendItemRightClicked(QwtPlotCurve*,QMouseEvent*);
    void axisPushRequested();
    void autoScaleAllRequested();

public slots:
    void legendItemClicked(QVariant info, bool checked, int index);
    void buildContextMenu(QMouseEvent *me);


private:

protected:
    void filterData();
    virtual bool eventFilter(QObject *obj, QEvent *ev);

};

#endif // DATAPLOT_H

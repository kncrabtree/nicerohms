#ifndef DATAPLOTVIEWWIDGET_H
#define DATAPLOTVIEWWIDGET_H

#include <QWidget>

#include <QList>

#include <qwt6/qwt_plot.h>

class QGridLayout;
class DataPlot;
class QwtPlotCurve;

class DataPlotViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataPlotViewWidget(QWidget *parent = 0);
    ~DataPlotViewWidget();

    //! Associates curves with which plot and axis they're displayed on
    struct CurveMetaData {
        QwtPlotCurve *curve; /*!< The curve */
        QVector<QPointF> data; /*!< The curve data */
        QString name; /*!< The name of the curve */
        int plotIndex; /*!< The index of the plot (in allPlots) the curve is plotted on */
        QwtPlot::Axis axis; /*!< The y-axis on which the curve is plotted */
        bool isVisible; /*!< Whether the curve is visible */
        double min;
        double max;
    };

public slots:
    void initializeForExperiment();
    void pointUpdated(const QList<QPair<QString,QVariant> > list, double x);
    void curveVisibilityToggled(QwtPlotCurve *c, bool visible);
    void curveContextMenuRequested(QwtPlotCurve *c, QMouseEvent *me);
    void changeCurveColor(int curveIndex);
    void moveCurveToPlot(int curveIndex, int newPlotIndex);
    void changeCurveAxis(int curveIndex);
    void pushXAxis(int sourcePlotIndex);
    void autoScaleAll();

    void changeNumPlots();


private:
    QGridLayout *d_gridLayout = nullptr;
    QList<CurveMetaData> d_plotCurves;
    QList<DataPlot*> d_allPlots;
    QPair<double,double> d_xRange;

    int findCurveIndex(QwtPlotCurve* c);
    void addNewPlot();
    void configureGrid();
    void setAutoScaleYRanges(int plotIndex, QwtPlot::Axis axis);

};

#endif // DATAPLOTVIEWWIDGET_H

#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>

#include <qwt_legend.h>
#include <qwt_picker_machine.h>

class Zoomer : public QwtPlotZoomer
{
public:
    explicit Zoomer(int, int ,QWidget*);
};

class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget *parent, const QString &legend);
    Plot(QWidget *parent, const QString &legend1, const QString &legend2);
    QwtPlotZoomer *plotZoomer[2];
    QwtPlotPicker *plotPicker;
    QwtPlotPanner *plotPanner;

    void showData(QPolygonF points, double amplitude, int curveNumber = 0);
    void showPeak(double, double);
    void setPlotLimits(double xMin = 350, double xMax = 1000, double yMin = 0, double yMax = 1000);
    void setxLabel(const QString &label);
    void setyLabel(const QString &label);

private:
    QwtPlotCurve *plotCurve[2];
    QwtPlotMarker *plotMarker;
    double yLimitMin;
    double yLimitMax;
    double xLimitMin;
    double xLimitMax;

    void setPlotDefaultParameters();

signals:
    void showInfo();

public slots:
    void enableZoomMode(bool on);
};

#endif // PLOT_H


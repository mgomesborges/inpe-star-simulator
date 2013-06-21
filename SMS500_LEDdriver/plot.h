#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include "zoomer.h"

class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent );
    QwtPlotZoomer *plotZoomer[2];
    QwtPlotPicker *plotPicker;
    QwtPlotPanner *plotPanner;

    void showData(QPolygonF&, double);
    void showPeak(double, double);
    void setPlotLimits(double xMin = 300, double xMax = 1100, double yMin = 0, double yMax = 1000);

private:
    QwtPlotCurve *plotCurve;
    QwtPlotMarker *plotMarker;
    double yLimitMin;
    double yLimitMax;
    double xLimitMin;
    double xLimitMax;

signals:
    void showInfo();

public slots:
    void enableZoomMode(bool on);
};

#endif // PLOT_H


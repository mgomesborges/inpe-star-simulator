#ifndef LONGTERMSTABILITYPLOT_H
#define LONGTERMSTABILITYPLOT_H

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

class LongTermStabilityPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit LongTermStabilityPlot(QWidget *parent = 0);

    void showData(QPolygonF& points, double amplitude);
    void setPlotLimits(double xMin = 300, double xMax = 1100, double yMin = 0, double yMax = 1000);

private:
    QwtPlotCurve *plotCurve[1];
    QwtPlotMarker *plotMarker;
    QwtPlotPanner *plotPanner;
    QwtPlotPicker *plotPicker;
    double yLimitMin;
    double yLimitMax;
    double xLimitMin;
    double xLimitMax;

signals:
    void showInfo();
};

#endif // LONGTERMSTABILITYPLOT_H

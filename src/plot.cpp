#include "plot.h"

Plot::Plot(QWidget *parent, const QString &legend) :
    QwtPlot(parent)
{
    plotCurve[0] = new QwtPlotCurve(legend);
    plotCurve[0]->setRenderHint(QwtPlotItem::RenderAntialiased);
    plotCurve[0]->setPen(QPen(Qt::red));
    plotCurve[0]->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    plotCurve[0]->setYAxis(QwtPlot::yLeft);
    plotCurve[0]->attach(this);

    setPlotDefaultParameters();
}

Plot::Plot(QWidget *parent, const QString &legend1, const QString &legend2) :
    QwtPlot(parent)
{
    plotCurve[0] = new QwtPlotCurve(legend1);
    plotCurve[0]->setRenderHint(QwtPlotItem::RenderAntialiased);
    plotCurve[0]->setPen(QPen(Qt::red));
    plotCurve[0]->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    plotCurve[0]->setYAxis(QwtPlot::yLeft);
    plotCurve[0]->attach(this);

    plotCurve[1] = new QwtPlotCurve(legend2);
    plotCurve[1]->setRenderHint(QwtPlotItem::RenderAntialiased);
    plotCurve[1]->setPen(QPen(Qt::blue));
    plotCurve[1]->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    plotCurve[1]->setYAxis(QwtPlot::yLeft);
    plotCurve[1]->attach(this);

    setPlotDefaultParameters();
}

void Plot::showData(QPolygonF points, double amplitude, int curveNumber)
{
    setAxisMaxMajor(QwtPlot::xBottom, 10);
    setAxisMaxMinor(QwtPlot::xBottom, 5);
    setAxisScale(QwtPlot::yLeft, 0, amplitude);
    setAxisScale(QwtPlot::xBottom, xLimitMin, xLimitMax, 100);
    plotCurve[curveNumber]->setSamples(points);
    this->replot();
}

void Plot::showPeak(double wavelength, double amplitude)
{
    setAxisScale(QwtPlot::yLeft, 0.0, amplitude);

    QwtText text(tr("Peak: %1").arg(amplitude));
    text.setFont(QFont( "Helvetica", 10, QFont::Bold));
    text.setColor(QColor(Qt::darkMagenta));
    plotMarker->setLabel(text);
    plotMarker->setValue(wavelength, amplitude);
}

void Plot::setPlotLimits(double xMin, double xMax, double yMin, double yMax)
{
    xLimitMin = xMin;
    xLimitMax = xMax;
    yLimitMin = yMin;
    yLimitMax = yMax;
}

void Plot::setxLabel(const QString &label)
{
    setAxisTitle( QwtPlot::xBottom, label);
}

void Plot::setyLabel(const QString &label)
{
    setAxisTitle(QwtPlot::yLeft, label);
}

void Plot::enableZoomMode(bool on)
{
    plotZoomer[0]->setEnabled(on);
    plotZoomer[0]->zoom( 0 );
    plotZoomer[1]->setEnabled( on );
    plotZoomer[1]->zoom( 0 );
    plotPanner->setEnabled(on);
    plotPicker->setEnabled(!on);
    setAxisScale(QwtPlot::xBottom, xLimitMin, xLimitMax);
    setAxisScale(QwtPlot::yLeft, yLimitMin, yLimitMax);
    emit showInfo();
}

void Plot::setPlotDefaultParameters()
{
    setAutoReplot(false);
    setCanvasBackground(QColor(Qt::white));

    // Legend
    QwtLegend *legend = new QwtLegend;
    insertLegend(legend, QwtPlot::BottomLegend);

    // Grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(QPen(Qt::white, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    // Marker
    plotMarker = new QwtPlotMarker();
    plotMarker->setValue(0.0, 0.0);
    plotMarker->setLineStyle(QwtPlotMarker::VLine);
    plotMarker->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom | Qt::AlignCenter);
    plotMarker->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
    plotMarker->attach(this);

    setAutoReplot(true);

    this->setContentsMargins(10, 10, 10, 10);

    // Zoomer
    plotZoomer[0] = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, this->canvas());
    plotZoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
    plotZoomer[0]->setRubberBandPen(QColor( Qt::green));
    plotZoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
    plotZoomer[0]->setTrackerPen(QColor( Qt::blue));

    plotZoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight, this->canvas());

    plotPanner = new QwtPlotPanner( this->canvas() );
    plotPanner->setMouseButton( Qt::MidButton );

    // Picker
    plotPicker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                   QwtPlotPicker::CrossRubberBand,
                                   QwtPicker::AlwaysOn, this->canvas());

    plotPicker->setStateMachine(new QwtPickerDragPointMachine());
    plotPicker->setRubberBand(QwtPicker::CrossRubberBand);
    plotPicker->setRubberBandPen(QColor(Qt::green));
    plotPicker->setTrackerPen(QColor(Qt::blue));

    setPlotLimits();
    enableZoomMode(false);
    this->show();
    emit showInfo();
}

Zoomer::Zoomer(int xAxis, int yAxis, QWidget *canvas) :
    QwtPlotZoomer(xAxis, yAxis, canvas)
{
    setTrackerMode( QwtPicker::AlwaysOff );
    setRubberBand( QwtPicker::NoRubberBand );

    // RightButton: zoom out by 1
    // Ctrl+RightButton: zoom out to full size

    setMousePattern( QwtEventPattern::MouseSelect2,
                     Qt::RightButton, Qt::ControlModifier );
    setMousePattern( QwtEventPattern::MouseSelect3,
                     Qt::RightButton );
}

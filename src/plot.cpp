#include "plot.h"

#include <qwt_legend.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_engine.h>


#include <qwt_math.h>
#include <qwt_symbol.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_text.h>
#include <qmath.h>


Plot::Plot( QWidget *parent ) : QwtPlot( parent )
{
    setAutoReplot( false );

//    setTitle( "No Title" );
    setCanvasBackground( QColor( Qt::white ) );

    // legend
    QwtLegend *legend = new QwtLegend;
    insertLegend( legend, QwtPlot::BottomLegend );

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajorPen( QPen( Qt::white, 0, Qt::DotLine ) );
    grid->setMinorPen( QPen( Qt::gray, 0 , Qt::DotLine ) );
    grid->attach( this );

    // axes
    setAxisTitle( QwtPlot::xBottom, "Wavelength (nm)" );
    setAxisTitle( QwtPlot::yLeft, "Amplitude (uW/nm)" );

    // curves
    plotCurve[0] = new QwtPlotCurve( "Amplitude" );
    plotCurve[0]->setRenderHint( QwtPlotItem::RenderAntialiased );
    plotCurve[0]->setPen( QPen( Qt::red ) );
    plotCurve[0]->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    plotCurve[0]->setYAxis( QwtPlot::yLeft );
    plotCurve[0]->attach( this );

    plotCurve[1] = new QwtPlotCurve( "Amplitude" );
    plotCurve[1]->setRenderHint( QwtPlotItem::RenderAntialiased );
    plotCurve[1]->setPen( QPen( Qt::blue ) );
    plotCurve[1]->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    plotCurve[1]->setYAxis( QwtPlot::yLeft );
    plotCurve[1]->attach( this );

    // marker
    plotMarker = new QwtPlotMarker();
    plotMarker->setValue( 0.0, 0.0 );
    plotMarker->setLineStyle( QwtPlotMarker::VLine );
    plotMarker->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom | Qt::AlignCenter);
    plotMarker->setLinePen( QPen( Qt::green, 0, Qt::DashDotLine ) );
    plotMarker->attach( this );

    setAutoReplot( true );

    const int margin = 10;
    this->setContentsMargins( margin, margin, margin, margin );

    plotZoomer[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, this->canvas() );
    plotZoomer[0]->setRubberBand( QwtPicker::RectRubberBand );
    plotZoomer[0]->setRubberBandPen( QColor( Qt::green ) );
    plotZoomer[0]->setTrackerMode( QwtPicker::ActiveOnly );
    plotZoomer[0]->setTrackerPen( QColor( Qt::blue ) );

    plotZoomer[1] = new Zoomer( QwtPlot::xTop, QwtPlot::yRight, this->canvas() );

    plotPanner = new QwtPlotPanner( this->canvas() );
    plotPanner->setMouseButton( Qt::MidButton );

    plotPicker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                    QwtPlotPicker::CrossRubberBand,
                                    QwtPicker::AlwaysOn, this->canvas() );

    plotPicker->setStateMachine( new QwtPickerDragPointMachine() );
    plotPicker->setRubberBand( QwtPicker::CrossRubberBand );
    plotPicker->setRubberBandPen( QColor( Qt::green ) );
    plotPicker->setTrackerPen( QColor( Qt::blue ) );

    setPlotLimits();

    this->show();
    enableZoomMode( false );
    emit showInfo();
}

void Plot::showData(QPolygonF &points, double amplitude)
{
    setAxisMaxMajor( QwtPlot::xBottom, 10 );
    setAxisMaxMinor( QwtPlot::xBottom, 5 );

//    setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine );
    setAxisScale( QwtPlot::yLeft, 0.0, amplitude  );
    setAxisScale( QwtPlot::xBottom, xLimitMin, xLimitMax, 100);
    plotCurve[0]->setSamples(points);
}

void Plot::showData(QPolygonF &points0, QPolygonF &points1, double amplitude)
{
    setAxisMaxMajor( QwtPlot::xBottom, 10 );
    setAxisMaxMinor( QwtPlot::xBottom, 5 );

    setAxisScale( QwtPlot::yLeft, 0.0, amplitude );
    setAxisScale( QwtPlot::xBottom, xLimitMin, xLimitMax, 100);
    plotCurve[0]->setSamples(points0);
    plotCurve[1]->setSamples(points1);
}

void Plot::showPeak(double wavelength, double amplitude)
{
    setAxisScale( QwtPlot::yLeft, 0.0, amplitude );

    QString label;
    label.sprintf( "Peak: %.3g", amplitude );

    QwtText text( label );
    text.setFont( QFont( "Helvetica", 10, QFont::Bold ) );
    text.setColor( QColor( Qt::darkMagenta ) );

    plotMarker->setValue( wavelength, amplitude );
    plotMarker->setLabel( text );
}

void Plot::setPlotLimits(double xMin, double xMax, double yMin, double yMax)
{
    xLimitMin = xMin;
    xLimitMax = xMax;
    yLimitMin = yMin;
    yLimitMax = yMax;
}

void Plot::enableZoomMode( bool on )
{
    plotPanner->setEnabled( on );

    plotZoomer[0]->setEnabled( on );
    plotZoomer[0]->zoom( 0 );

    plotZoomer[1]->setEnabled( on );
    plotZoomer[1]->zoom( 0 );

    plotPicker->setEnabled( !on );

    setAxisScale(QwtPlot::xBottom, xLimitMin, xLimitMax);
    setAxisScale(QwtPlot::yLeft, yLimitMin, yLimitMax);

    emit showInfo();
}

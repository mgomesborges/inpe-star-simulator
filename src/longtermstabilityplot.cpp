#include "longtermstabilityplot.h"

LongTermStabilityPlot::LongTermStabilityPlot(QWidget *parent) :
    QwtPlot(parent)
{
    setAutoReplot( false );

    setCanvasBackground( QColor( Qt::white ) );

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajorPen( QPen( Qt::white, 0, Qt::DotLine ) );
    grid->setMinorPen( QPen( Qt::gray, 0 , Qt::DotLine ) );
    grid->attach( this );

    // curves
    plotCurve[0] = new QwtPlotCurve( "Amplitude" );
    plotCurve[0]->setRenderHint( QwtPlotItem::RenderAntialiased );
    plotCurve[0]->setPen( QPen( Qt::red ) );
    plotCurve[0]->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    plotCurve[0]->setYAxis( QwtPlot::yLeft );
    plotCurve[0]->attach( this );

    plotPanner = new QwtPlotPanner( this->canvas() );
    plotPanner->setMouseButton( Qt::MidButton );

    plotPicker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                    QwtPlotPicker::CrossRubberBand,
                                    QwtPicker::AlwaysOn, this->canvas() );

    plotPicker->setStateMachine( new QwtPickerDragPointMachine() );
    plotPicker->setRubberBand( QwtPicker::CrossRubberBand );
    plotPicker->setRubberBandPen( QColor( Qt::green ) );
    plotPicker->setTrackerPen( QColor( Qt::blue ) );

    setAutoReplot( true );
    const int margin = 10;
    this->setContentsMargins( margin, margin, margin, margin );
    setPlotLimits();
    this->show();
}

void LongTermStabilityPlot::showData(QPolygonF &points, double amplitude)
{
    setAxisMaxMajor( QwtPlot::xBottom, 10 );
    setAxisMaxMinor( QwtPlot::xBottom, 5 );

    setAxisScale( QwtPlot::yLeft, 0.0, amplitude );
    setAxisScale( QwtPlot::xBottom, xLimitMin, xLimitMax, 100);
    plotCurve[0]->setSamples(points);
}

void LongTermStabilityPlot::setPlotLimits(double xMin, double xMax, double yMin, double yMax)
{
    xLimitMin = xMin;
    xLimitMax = xMax;
    yLimitMin = yMin;
    yLimitMax = yMax;
}

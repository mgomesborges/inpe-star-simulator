#include "zoomer.h"

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

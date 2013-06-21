#ifndef ZOOMER_H
#define ZOOMER_H

#include <qwt_plot_zoomer.h>

class Zoomer : public QwtPlotZoomer
{
public:
    explicit Zoomer(int, int ,QwtPlotCanvas*);
};

#endif // ZOOMER_H

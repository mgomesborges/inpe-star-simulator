#ifndef ZOOMER_H
#define ZOOMER_H

#include <qwt_plot_zoomer.h>

class Zoomer : public QwtPlotZoomer
{
public:
    explicit Zoomer(int, int ,QWidget*);
};

#endif // ZOOMER_H

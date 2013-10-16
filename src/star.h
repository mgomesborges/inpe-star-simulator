#ifndef STAR_H
#define STAR_H

#include <QObject>
#include <QVector>
#include <QPolygon>
#include <cmath>

class Star : public QObject
{
    Q_OBJECT
public:
    explicit Star(QObject *parent = 0);

    int magnitude();
    int temperature();
    double peak();
    void setMagnitude(int magnitude);
    void setTemperature(int temperature);

    QVector< QVector<double> > spectralData();
    QPolygonF spectralDataToPlot();

private:
    int starMagnitude;
    int starTemperature;
    double starPeak;
};

#endif // STAR_H

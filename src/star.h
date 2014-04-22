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

    double magnitude();
    int temperature();
    double peak();
    void setMagnitude(double magnitude);
    void setTemperature(int temperature);

    QVector< QVector<double> > spectralData();
    QPolygonF spectralDataToPlot();

private:
    double starMagnitude;
    int starTemperature;
    double starPeak;
};

#endif // STAR_H

#include "star.h"

Star::Star(QObject *parent) :
    QObject(parent)
{
    starMagnitude = 0;
    starTemperature = 7500;
}

double Star::magnitude()
{
    return starMagnitude;
}

int Star::temperature()
{
    return starTemperature;
}

double Star::peak()
{
    return starPeak;
}

void Star::setMagnitude(double magnitude)
{
    starMagnitude = magnitude;
}

void Star::setTemperature(int temperature)
{
    starTemperature = temperature;
}

QVector<QVector<double> > Star::spectralData()
{
    // Spectral Power Distribution of a model Star (W cm^-2 nm^-1)
    //
    // Allen, C. W. Astrophysical Quantities. Great Britain: Willian Clowes,
    // 1973. 197-209.
    //
    // Matos, Jose Dias. Dimensionamento radiometrico preliminar do sensor de estrelas.
    // Sao Jose dos Campos: INPE, 1997. Relatorio Tecnico.

    QVector< QVector<double> > starData;
    double WL[641]; // Size = [360, 1000]nm = 641

    // Important constants
    double k = 1.380650424e-23; // Boltzmann's constant J/k
    double h = 6.62606896e-34;  // Planck's constant Js
    double c = 299792458;       // Speed of light m/s

    // Inits LED Modeling Data structure
    starData.resize(641); // Size = 1000nm - 360nm = 640
    for (int i = 0; i < 641; i++) {
        starData[i].resize(2);
        starData[i][0] = i + 360;

        // Converts Wavelengths in nanometers to meters
        WL[i] = (i + 360) * 1e-9;

        // Blackbody in W m^-2 m^-1
        starData[i][1] = (2 * M_PI * h * pow(c,2)) / (pow(WL[i],5) * (exp((h * c) / (k * WL[i] * starTemperature)) - 1));

        // Converts (W m^-2 m^-1) to (W cm^-2 nm^-1)
        starData[i][1] = starData[i][1] / 1e13;
    }

    // Finds irradiance value at 550nm [wavelength = 360 : 1 : 1000]
    int index550 = 190;

    // Calculates Apparent Magnitude Transference Function
    double visualMagnitude     = pow(10,(-0.4 * starMagnitude)) * 4e-15;
    double conversionFactor550 = visualMagnitude / starData[index550][1];

    // Resets star peak
    starPeak = 0;

    // Spectral Irradiance of a model Star (uW cm^-2 nm^-1)
    for (int i = 0; i < 641; i++) {
        // Converts W in uW: 1W = 1 x 10^6 uW
        starData[i][1] = starData[i][1] * conversionFactor550 * 1e6;

        // Get star's peak value
        if (starData[i][1] > starPeak) {
            starPeak = starData[i][1];
        }
    }

    return starData;
}

QPolygonF Star::spectralDataToPlot()
{
    QPolygonF spectralDataToPlot;
    QVector< QVector<double> > starData = spectralData();

    // Prepares Spectral Data to plot
    for (int i = 0; i < 641; i++) {
        spectralDataToPlot << QPointF(starData[i][0], starData[i][1]);
    }
    return spectralDataToPlot;
}

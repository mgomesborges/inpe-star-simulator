#include "star.h"
#include "ui_star.h"

Star::Star(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Star)
{
    ui->setupUi(this);
}

Star::~Star()
{
    delete ui;
}

void Star::setMagnitude(const QString &magnitude) const
{
    ui->magnitude->setText(magnitude);
}

void Star::setTemperature(const QString & temperature) const
{
    ui->temperature->setText(temperature);
}

int Star::magnitude()
{
    return ui->magnitude->text().toInt();
}

int Star::temperature()
{
    return ui->temperature->text().toInt();
}

std::vector< std::vector<double> > Star::getData(int magnitude, int temperature)
{
    // Spectral Power Distribution of a model Star (W/cm^2/nm)
    //
    // Allen, C. W. Astrophysical Quantities. Great Britain: Willian Clowes,
    // 1973. 197-209.
    //
    // Matos, José Dias. Dimensionamento radiométrico preliminar do sensor de estrelas.
    // São José dos Campos: INPE, 1997. Relatório Técnico.

    std::vector< std::vector<double> > starData;
    double WL[641]; // Size = 1000nm - 360nm = 640

    // Important constants
    double k = 1.380650424e-23; // Boltzmann's constant J/k
    double h = 6.62606896e-34;  // Planck's constant Js
    double c = 299792458;       // Speed of light m/s

    // Inits LED Modeling Data structure
    starData.resize( 641 ); // Size = 1000nm - 360nm = 640
    for (int i = 0; i <= 640; i++) {
        starData[i].resize( 2 );
        starData[i][0] = i + 360;

        // Converts Wavelengths in nanometers to meters
        WL[i] = (i + 360) * 1e-9;

        // Blackbody in W/m^2/m
        starData[i][1] = (2 * M_PI * h * pow(c,2)) / (pow(WL[i],5) * (exp((h * c) / (k * WL[i] * temperature)) - 1));
    }

    // Finds irradiance value at 550nm [wavelength = 360 : 1 : 1000]
    int index550 = 190;

    // Calculates Apparent Magnitude Transference Function
    double visualMagnitude = pow(10,(-0.4 * magnitude)) * 4e-15;

    double conversionFactor550 = visualMagnitude / starData[index550][1];

    // Transference Function of Pinhole and Colimator
    // 1e-6 is conversion of watt to microwatt
    double transferenceFunction = 7.2035e-12 * 1e-6;

    for (int i = 0; i <= 640; i++) {
        // Spectral Irradiance of a model Star (W/cm^2/nm)
        starData[i][1] = starData[i][1] * conversionFactor550 / transferenceFunction;
    }

    return starData;
}

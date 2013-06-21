#include "sms500.h"

SMS500::SMS500(QObject *parent, int smsChannel) :
    QThread(parent)
{
    channel = smsChannel;
    enableNoiseReduction = false;
    spectrometer.Channel[channel].MaxSpCounts  = 65535;
    spectrometer.Channel[channel].SetMaxCounts = 52800;
    spectrometer.Channel[channel].SetMinCounts = 24000;
}

SMS500::~SMS500()
{
    closeConnection();
}

void SMS500::run()
{
    enabledScan     = true;
    enabledNextScan = false;
    double amplitude;
    int peakWavelength;
    int scanNumber = 0;
    int integrationTimeIndex = 0;
    bool needAutoScan = true;

    ReadSpecDataFile(calibratedDataPath.toAscii().data());

    while (enabledScan == true) {
        scanNumber++;

        EnableNoiseReduction(enableNoiseReduction, noiseReductionFator);

        if ((autoRange == true) && (needAutoScan == true)) {
            integrationTimeIndex = performAutoRange();
            setRange(integrationTimeIndex);
            GetSpectralData(&spectrometer, &resultData, 0);
        } else {
            GetSpectralData(&spectrometer, &resultData, 0);
        }

        amplitude      = -1000;
        peakWavelength = 0;

        for(int i = 0; i < resultData.Points; i++) {
            if (amplitude < resultData.MasterData[i]) {
                amplitude      = resultData.MasterData[i];
                peakWavelength = resultData.WaveLength[i];
            }
        }

        needAutoScan = isNeedAutoScan();

        emit scanPerformed(resultData.MasterData, resultData.WaveLength, peakWavelength,
                           amplitude, resultData.Points, scanNumber, integrationTimeIndex, satured);

        if ((numberOfScans != -1) && (scanNumber >= numberOfScans)) {
            break;
        }

        while ((enabledScan == true) && (enabledNextScan == false));
    }

    emit scanFinished();
}

void SMS500::stop()
{
    enabledScan = false;
}

void SMS500::setOperationMode(const QString &mode)
{
    spectrometer.Channel[channel].pchModeType = mode.toAscii().data();
}

void SMS500::setCalibratedDataPath(const QString &path)
{
    calibratedDataPath = path;
}

void SMS500::setAutoRange(bool enable)
{
    // Do not use the SMS500 Autorange function, but uses the own implemented autorange
    spectrometer.Channel[channel].AutoRang = false;
    autoRange = enable;
}

void SMS500::setRange(int range)
{
    spectrometer.Channel[channel].Range = range;
    spectrometer.Channel[channel].IntTime = INTEGRATION_TIME[range];
}

void SMS500::setBoxCarSmoothing(short value)
{
    spectrometer.Channel[channel].BoxCar = value;
}

void SMS500::setAverage(short average)
{
    spectrometer.Channel[channel].Aveg = average;
}

void SMS500::setStartWave(int wave)
{
    spectrometer.Channel[channel].StartWave = wave;
}

void SMS500::setStopWave(int wave)
{
    spectrometer.Channel[channel].StopWave = wave;
}

void SMS500::setCorrecDarkCurrent(bool enable)
{
    spectrometer.Channel[channel].CorrDark = enable;
}

void SMS500::setNoiseReduction(bool enable, double factor)
{
    enableNoiseReduction = enable;
    noiseReductionFator  = factor;
}

void SMS500::setNumberOfScans(int value)
{
    numberOfScans = value;
}

int SMS500::startWavelength()
{
    return resultData.Start;
}

int SMS500::stopWavelength()
{
    return resultData.Stop;
}

int SMS500::dominanteWavelength()
{
    return resultData.Domwave;
}

int SMS500::peakWavelength()
{
    return resultData.Peakwave;
}

int SMS500::fwhm()
{
    return resultData.FWHMWL;
}

int SMS500::points()
{
    return resultData.Points;
}

double SMS500::power()
{
    return resultData.Power;
}

double SMS500::integrationTime()
{
    return resultData.IntgTime;
}

float SMS500::purity()
{
    return resultData.Purity;
}

double *SMS500::masterData()
{
    return resultData.MasterData;
}

int *SMS500::wavelength()
{
    return resultData.WaveLength;
}

double SMS500::dllVersion()
{
    return GetSpecDLLVersion();
}

char *SMS500::serialNumber()
{
    return spectrometer.Channel[channel].SerialNum;
}

double SMS500::coefficient1()
{
    return spectrometer.Channel[0].Cof1;
}

double SMS500::coefficient2()
{
    return spectrometer.Channel[0].Cof2;
}

double SMS500::coefficient3()
{
    return spectrometer.Channel[0].Cof3;
}

double SMS500::intercept()
{
    return spectrometer.Channel[0].Intercept;
}

bool SMS500::readCalibratedLamp(const QString &path)
{
    return ReadCalStd(path.toAscii().data());
}

void SMS500::startLampScan()
{
    StartLampScan(&spectrometer, 0);
}

void SMS500::finishLampScan()
{
    FinishLampScan(&spectrometer, calibratedDataPath.toAscii().data(), 0);
}

void SMS500::creatDarkRatioArray()
{
    CreateDarkRatioArray(&spectrometer);
}

bool SMS500::openConnection()
{
    if (FindInitSpectrometer(&spectrometer) == 16) {
        return true; // Successfully connected
    }
    return false;
}

void SMS500::closeConnection()
{
    stop();
    wait();
    CloseSpectrometer();
    spectrometer.Channel[channel].SpectType = SPEC_NOT_FOUND; //Spectrometer hardware not connected
}

bool SMS500::isConnected()
{
    if (spectrometer.Channel[channel].SpectType == 16) {
        return true;
    }
    return false;
}

bool SMS500::isNeedAutoScan()
{
    double max = 0;

    for (int i = 0; i < resultData.Points; i++) {
        if (max < resultData.PixelValues[i]) {
            max = resultData.PixelValues[i];
        }
    }

    if (max > spectrometer.Channel[channel].SetMaxCounts) {
        satured = true;
    } else {
        satured = false;
    }

    if ((max > spectrometer.Channel[channel].SetMaxCounts) || (max < spectrometer.Channel[channel].SetMinCounts)) {
        return true;
    }

    return false;
}

int SMS500::performAutoRange()
{
    double max;

    for (int i = 1; i < MAX_RANGES; i++) {
        setRange(i);
        GetSpectralData( &spectrometer, &resultData, 0);

        max = 0;

        for (int j = 0; j < resultData.Points; j++) {
            if (max < resultData.PixelValues[j]) {
                max = resultData.PixelValues[j];
            }
        }

        // Satured
        if (max > spectrometer.Channel[channel].SetMaxCounts) {
            return (i - 1);
        }
    }

    return (MAX_RANGES -1);
}

void SMS500::enableNextScan()
{
    enabledNextScan = true;
}

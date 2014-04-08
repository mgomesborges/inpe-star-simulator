#include "sms500.h"

SMS500::SMS500(QObject *parent, int smsChannel) :
    QThread(parent)
{
    channel = smsChannel;
    enableNoiseReduction = false;
    spectrometer.Channel[channel].MaxSpCounts  = 65535;
    spectrometer.Channel[channel].SetMaxCounts = 52800;
    spectrometer.Channel[channel].SetMinCounts = 24000;

    sleepSingleShot   = false;
    millisecondsSleep = 0;
    scanNumber        = 0;
}

SMS500::~SMS500()
{
    closeConnection();
}

void SMS500::run()
{
    SPECTROMETER currentSpectrometer;

    ReadSpecDataFile(calibratedDataPath.toUtf8().data());

    enabledScan = true;
    scanNumber  = 0;

    while (enabledScan == true) {
        // Sleep
        msleep(millisecondsSleep);
        if (sleepSingleShot == true) {
            millisecondsSleep = 0;
        }

        // Update spectrometer parameters
        currentSpectrometer = spectrometer;

        scanNumber++;

        EnableNoiseReduction(enableNoiseReduction, noiseReductionFator);
        GetSpectralData(&currentSpectrometer, &resultData, 0);

        if ((isNeedAutoRange() == true) && (autoRange == true)) {
            performAutoRange();
        }

        enabledNextScan = false;

        emit scanPerformed(scanNumber);

        // Waiting for SMS500 be ready for next scan
        while ((enabledScan == true) && (enabledNextScan == false)) {
            msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        }

        if ((numberOfScans != -1) && (scanNumber >= numberOfScans)) {
            break;
        }
    }

    // Reset Sleep
    sleepSingleShot   = false;
    millisecondsSleep = 0;

    emit scanFinished();
}

void SMS500::stop()
{
    enabledScan = false;
}

void SMS500::setOperationMode(operationMode mode)
{
    switch (mode) {
        case Flux:
            spectrometer.Channel[channel].pchModeType = QString("Flux").toUtf8().data();
            calibratedDataPath = "Flux.dat";
            break;
        case Intensity:
            spectrometer.Channel[channel].pchModeType = QString("Intensity").toUtf8().data();
            calibratedDataPath = "Irradiance.dat";
            break;
        case lux:
            spectrometer.Channel[channel].pchModeType = QString("lux").toUtf8().data();
            calibratedDataPath = "Irradiance.dat";
            break;
        case cd_m2:
            spectrometer.Channel[channel].pchModeType = QString("cd/m2").toUtf8().data();
            calibratedDataPath = "Radiance.dat";
            break;
    }
}

void SMS500::setAutoRange(bool enable)
{
    // This class don't use the SMS500 Autorange function, but uses the own implemented autorange
    spectrometer.Channel[channel].AutoRang = false;
    autoRange = enable;
}

void SMS500::setRange(int rangeIndex)
{
    spectrometer.Channel[channel].Range = rangeIndex;
    spectrometer.Channel[channel].IntTime = INTEGRATION_TIME[rangeIndex];
    emit integrationTimeChanged(rangeIndex);
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

void SMS500::resetScanNumber()
{
    scanNumber = 0;
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

double SMS500::maxIntensity()
{
    double max = 0;

    for (int i = 0; i < resultData.Points; i++) {
        if (max < resultData.PixelValues[i]) {
            max = resultData.PixelValues[i];
        }
    }

    return max;
}

double SMS500::maxMasterData()
{
    return resultData.MasterData[resultData.Peakwave - resultData.Start];
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

double SMS500::samplesToAverage()
{
    return spectrometer.Channel[channel].Aveg;
}

double SMS500::boxCarSmoothing()
{
    return spectrometer.Channel[channel].BoxCar;
}

bool SMS500::isNoiseReductionEnabled()
{
    return enableNoiseReduction;
}

int SMS500::noiseReduction()
{
    return noiseReductionFator;
}

bool SMS500::isDynamicDarkEnabled()
{
    return spectrometer.Channel[channel].CorrDark;
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
    return ReadCalStd(path.toUtf8().data());
}

void SMS500::startLampScan()
{
    StartLampScan(&spectrometer, 0);
}

void SMS500::finishLampScan()
{
    FinishLampScan(&spectrometer, calibratedDataPath.toUtf8().data(), 0);
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
    wait(1000);
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

bool SMS500::isNeedAutoRange()
{
    double max = maxIntensity();

    if (max > spectrometer.Channel[channel].SetMaxCounts) {
        emit saturedData(true);
    } else {
        emit saturedData(false);
    }

    if (((max > spectrometer.Channel[channel].SetMaxCounts) && (spectrometer.Channel[channel].IntTime != 1.1)) ||
            ((max < spectrometer.Channel[channel].SetMinCounts) && (spectrometer.Channel[channel].IntTime != 4000))) {
        return true;
    }

    return false;
}

int SMS500::performAutoRange()
{
    SPECTROMETER currentSpectrometer = spectrometer;

    currentSpectrometer.Channel[channel].Aveg = 1;
    currentSpectrometer.Channel[channel].BoxCar = 1;
    currentSpectrometer.Channel[channel].CorrDark = false;

    EnableNoiseReduction(false, 0.0);

    int rangeIndex = currentSpectrometer.Channel[channel].Range;

    // If Satured
    if (maxIntensity() > currentSpectrometer.Channel[channel].SetMaxCounts) {
        for (; rangeIndex > 0; --rangeIndex) {
            currentSpectrometer.Channel[channel].Range = rangeIndex;
            currentSpectrometer.Channel[channel].IntTime = INTEGRATION_TIME[rangeIndex];

            GetSpectralData(&currentSpectrometer, &resultData, 0);

            if (maxIntensity() < currentSpectrometer.Channel[channel].SetMaxCounts) {
                break;
            }
        }
    } else { // If Not Satured
        for (; rangeIndex < MAX_RANGES; ++rangeIndex) {
            currentSpectrometer.Channel[channel].Range = rangeIndex;
            currentSpectrometer.Channel[channel].IntTime = INTEGRATION_TIME[rangeIndex];

            GetSpectralData(&currentSpectrometer, &resultData, 0);

            if (maxIntensity() > currentSpectrometer.Channel[channel].SetMaxCounts) {
                break;
            }
        }
        rangeIndex--;
    }

    setRange(rangeIndex);
    currentSpectrometer = spectrometer;
    EnableNoiseReduction(enableNoiseReduction, noiseReductionFator);
    GetSpectralData(&currentSpectrometer, &resultData, 0);
    return rangeIndex;
}

void SMS500::enableNextScan()
{
    enabledNextScan = true;
}

void SMS500::setWaitTimeForScan(unsigned long milliseconds)
{
    sleepSingleShot   = false;
    millisecondsSleep = milliseconds;
}

void SMS500::setWaitTimeForScanSingleShot(unsigned long milliseconds)
{
    sleepSingleShot   = true;
    millisecondsSleep = milliseconds;
}

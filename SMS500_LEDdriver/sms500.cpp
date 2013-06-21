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
    double amplitude;
    int peakWavelength;
    int integrationTimeIndex = 0;
    bool needAutoScan = true;

    ReadSpecDataFile(calibratedDataPath.toAscii().data());

    enabledScan = true;
    scanNumber  = 0;

    while (enabledScan == true) {
        // Sleep
        msleep(millisecondsSleep);
        if (sleepSingleShot == true) {
            millisecondsSleep = 0;
        }

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

        enabledNextScan = false;

        emit scanPerformed(resultData.MasterData, resultData.WaveLength, peakWavelength,
                           amplitude, resultData.Points, scanNumber, integrationTimeIndex, satured);

        // Waiting for SMS500 be ready for next scan
        while ((enabledScan == true) && (enabledNextScan == false));

        if ((numberOfScans != -1) && (scanNumber >= numberOfScans)) {
            sleepSingleShot   = false;
            millisecondsSleep = 0;
            break;
        }
    }

    emit scanFinished();
}

void SMS500::stop()
{
    enabledScan       = false;
    sleepSingleShot   = false;
    millisecondsSleep = 0;
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
    double max = 0;

    for (int i = 0; i < resultData.Points; i++) {
        if (max < resultData.MasterData[i]) {
            max = resultData.MasterData[i];
        }
    }

    return max;
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

bool SMS500::isNeedAutoScan()
{
    double max = maxIntensity();

    if (max > spectrometer.Channel[channel].SetMaxCounts) {
        satured = true;
    } else {
        satured = false;
    }

    if (((max > spectrometer.Channel[channel].SetMaxCounts) && (spectrometer.Channel[channel].IntTime != 1.1)) ||
            ((max < spectrometer.Channel[channel].SetMinCounts) && (spectrometer.Channel[channel].IntTime != 4000))) {
        return true;
    }

    return false;
}

int SMS500::performAutoRange()
{
    short average           = spectrometer.Channel[channel].Aveg;
    short boxCarSmoothing   = spectrometer.Channel[channel].BoxCar;
    bool correctDarkCurrent = spectrometer.Channel[channel].CorrDark;
    bool noiseReduction     = enableNoiseReduction;


    // Set Parameters
    spectrometer.Channel[channel].Aveg = 1;
    spectrometer.Channel[channel].BoxCar = 0;
    spectrometer.Channel[channel].CorrDark = false;
    enableNoiseReduction = false;

    // Performs first scan
    if (scanNumber <= 1) {
        GetSpectralData( &spectrometer, &resultData, 0);
    }

    double max = maxIntensity();
    int range  = spectrometer.Channel[channel].Range;

    // If Satured
    if (max > spectrometer.Channel[channel].SetMaxCounts) {
        for (; range > 0; --range) {
            setRange( range );
            GetSpectralData( &spectrometer, &resultData, 0);

            max = maxIntensity();

            if (max < spectrometer.Channel[channel].SetMaxCounts) {
                break;
            }
        }
    } else { // If Not Satured
        for (; range < MAX_RANGES; ++range) {
            setRange( range );
            GetSpectralData( &spectrometer, &resultData, 0);

            max = maxIntensity();

            if (max > spectrometer.Channel[channel].SetMaxCounts) {
                break;
            }
        }
        range -= 1;
    }

    // Reset Parameters
    spectrometer.Channel[channel].Aveg = average;
    spectrometer.Channel[channel].BoxCar = boxCarSmoothing;
    spectrometer.Channel[channel].CorrDark = correctDarkCurrent;
    enableNoiseReduction = noiseReduction;

    return range;
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

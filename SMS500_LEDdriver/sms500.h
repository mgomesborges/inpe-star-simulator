#ifndef SMS500_H
#define SMS500_H

#include <QThread>
#include "SpecData.h"

class SMS500 : public QThread
{
    Q_OBJECT

public:
    explicit SMS500(QObject *parent = 0, int smsChannel = 0);
    ~SMS500();

    void stop();

    void setOperationMode(const QString &mode);
    void setCalibratedDataPath(const QString &path);
    void setAutoRange(bool enable);
    void setRange(int range);
    void setBoxCarSmoothing(short value);
    void setAverage(short average);
    void setStartWave(int wave);
    void setStopWave(int wave);
    void setCorrecDarkCurrent(bool enable);
    void setNoiseReduction(bool enable, double factor);
    void setNumberOfScans(int value);
    int  startWavelength();
    int  stopWavelength();
    int  dominanteWavelength();
    int  peakWavelength();
    int  fwhm();
    int  points();
    double power();
    double integrationTime();
    double samplesToAverage();
    double boxCarSmoothing();
    float  purity();
    double* masterData();
    int* wavelength();
    double dllVersion();
    char *serialNumber();
    double coefficient1();
    double coefficient2();
    double coefficient3();
    double intercept();
    double maxIntensity();
    double maxMasterData();
    bool readCalibratedLamp(const QString &path);
    void startLampScan();
    void finishLampScan();
    void creatDarkRatioArray();
    bool openConnection();
    void closeConnection();
    bool isConnected();
    bool isNeedAutoScan();
    int  performAutoRange();
    void enableNextScan();
    void setWaitTimeForScan(unsigned long milliseconds);
    void setWaitTimeForScanSingleShot(unsigned long milliseconds);

private:
    void run();

signals:
    void scanPerformed(const double *masterData, const int *wavelegth, int peakWavelength,
                       double amplitude, int numberOfPoints, int scanNumber, int integrationTimeIndex, bool satured);
    void scanFinished();

public slots:

private:
    SPECTROMETER spectrometer;
    RESULTDATA resultData;
    QString calibratedDataPath;
    double noiseReductionFator;
    bool enableNoiseReduction;
    bool enabledScan;
    bool enabledNextScan;
    bool satured;
    bool autoRange;
    int numberOfScans;
    int scanNumber;
    int channel;

    unsigned long millisecondsSleep;
    bool sleepSingleShot;
};

#endif // SMS500_H

#ifndef DATATYPE_H
#define DATATYPE_H

#include <QVector>

typedef enum SMS500ModeType {
    Flux = 0,
    Intensity,
    Lux,
    Cd_m2
} SMS500OperationMode;

struct SMS500Parameters {
    SMS500OperationMode operationMode;
    int numberOfScans;
    bool autoRange;
    int integrationTime;
    int samplesToAverage;
    int boxCarSmoothing;
    int startWave;
    int stopWave;
    bool noiseReduction;
    double noiseReductionFactor;
    bool dynamicDark;

    // Default values
    SMS500Parameters()
        : operationMode(Flux)
        , numberOfScans(-1)
        , autoRange(false)
        , integrationTime(4)
        , samplesToAverage(1)
        , boxCarSmoothing(1)
        , startWave(360)
        , stopWave(1000)
        , noiseReduction(false)
        , noiseReductionFactor(1.0)
        , dynamicDark(false)
    {}
};

struct RemoteControlParameters {
    int tcpPort;

    // Default values
    RemoteControlParameters()
        : tcpPort(6000)
    {}
};

struct StarSimulatorParameters {
    double lmDampingFactor;
    int lmMaxIteration;
    double gdDampingFactor;
    int gdMaxIteration;
    double ofFactor;

    // Default values
    StarSimulatorParameters()
        : lmDampingFactor(1.5)
        , lmMaxIteration(20)
        , gdDampingFactor(0.05)
        , gdMaxIteration(10)
        , ofFactor(1e10)
    {}
};

#endif // DATATYPE_H

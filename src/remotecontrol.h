#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "datatype.h"

class RemoteControl : public QObject
{
    Q_OBJECT
public:
    enum errorCodes {
        UNDEFINED = -1,
        SUCCESS = 0,
        SOCKET_NULL,
        CONNECTION_REFUSED,
        INVALID_COMMAND,
        ERROR_WRONG_PARAMETERS,
        WITHOUT_ANSWER,
        WRONG_RETURN
    };

    enum algorithmStatus {
        FITING_OK = 0,
        PERFORMING_FITING,
        STOPPED
    };

    explicit RemoteControl(QObject *parent = 0);
    void listen();

signals:
    void setSMS500AutoRange(bool enable);
    void SMS500Connect();
    void SMS500Disconnect();
    void SMS500StartScan();
    void SMS500StopScan();
    void setSMS500DefaultSettings(SMS500Parameters settings);
    void setSMS500NumberOfScans(QString value);
    void setSMS500IntegrationTime(int index);
    void setSMS500SamplesToAverage(int value);
    void setSMS500BoxcarSmothing(int value);
    void setSMS500NoiseReduction(bool enable);
    void setSMS500NoiseReductionFactor(QString value);
    void setSMS500CorrectForDynamicDark(bool enable);
    void LEDDriverConnect();
    void LEDDriverDisconnect();
    void startLEDDriverModeling();
    void stopLEDDriverModeling();
    void setLEDDriverV2Ref(bool enable);
    void loadLEDDriverValues(QVector<double> values);
    void warningMessage(const QString &caption, const QString &message);
    void setStarMagnitude(QString value);
    void setStarTemperature(QString value);
    void setStarSimulatorAlgorithmLM(bool enable);
    void setStarSimulatorAlgorithmGD(bool enable);
    void setStarSimulatorX0random(bool enable);
    void setStarSimulatorX0userDefined(bool enable);
    void setStarSimulatorX0ledDriver(bool enable);
    void starSimulatorLoadInitialSolution(QVector<double> values);
    void startStarSimulator();
    void stopStarSimulator();
    void starSimulatorStatus();
    void starSimulatorIrradiances();

public slots:
    void newConnection();
    void readyRead();
    void disconnected();
    void setPort(int port);
    bool isConnected();
    void sendAnswer(int errorCode);
    void sendAnswer(QString data);

private:
    QTcpServer* server;
    QTcpSocket* socket;
    int numberOfConnections;
    int port;
};

#endif // REMOTECONTROL_H

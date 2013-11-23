#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <QThread>
#include <Qtime>
#include <QVector>

#include <Windows.h>
#include "ftd2xx.h"

class LedDriver : public QThread
{
    Q_OBJECT
public:
    enum modelingParameters {
        levelIncrement,
        levelDecrement
    };

    explicit LedDriver(QObject *parent = 0);
    ~LedDriver();

    void setModelingParameters(int startChannelValue       = 25,
                               int endChannelValue         = 26,
                               int levelUpdateType         = levelIncrement,
                               int incrementDecrementValue = 50);
    void modelingNextChannel();
    QVector<int> digitalLevelIndex();


signals:
    void warningMessage(QString title, QString message);
    void performScan();
    void saveData(QString fileName);
    void saveAllData(QString atualChannel);
    void modelingFinished();
    void info(QString message);

public slots:
    void stop();
    bool openConnection();
    void closeConnection();
    bool isConnected();
    void enabledModelingContinue();
    bool writeData(const char *txBuffer);
    bool resetDACs();
    bool configureVoltageRef();
    bool setV2Ref(bool checked);

private:
    void run();

    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;
    bool connected;
    bool v2ref;
    int startChannel;
    int endChannel;
    int incDecValue;
    int updateType;
    bool enabledModeling;
    bool enabledContinue;
    bool nextChannel;
    QVector<int> levelIndex;
};

#endif // LEDDRIVER_H

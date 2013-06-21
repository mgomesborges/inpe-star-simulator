#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <QThread>
#include <Qtime>

#include <Windows.h>
#include "ftd2xx.h"

class LedDriver : public QThread
{
    Q_OBJECT
public:
    explicit LedDriver(QObject *parent = 0);
    ~LedDriver();

    void setModelingParameters(int startChannelValue   = 25,
                               int endChannelValue     = 11,
                               int levelDecrementValue = 50);
    void modelingNextChannel();

    
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

private:
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;
    bool connected;

    int startChannel;
    int endChannel;
    int levelDecrement;
    bool enabledModeling;
    bool enabledContinue;
    bool nextChannel;

    void run();
};

#endif // LEDDRIVER_H

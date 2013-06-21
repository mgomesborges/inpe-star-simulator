#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <QThread>
#include <Windows.h>
#include "ftd2xx.h"

class LedDriver : public QThread
{
    Q_OBJECT
public:
    explicit LedDriver(QObject *parent = 0);
    ~LedDriver();

    void setModelingParameters(int startDAC       =    0,
                               int endDAC         =   11,
                               int startPort      =    0,
                               int endPort        =    7,
                               int startLevel     =    0,
                               int endLevel       = 4095,
                               int incrementLevel =   10);

    
signals:
    void warningMessage(QString title, QString message);
    void performScan();
    void saveData(QString fileName);
    void modelingFinished();
    
public slots:
    void stop();
    bool openConnection();
    void closeConnection();
    bool isConnected();
    void enabledModelingContinue();
    bool writeData(const char *txBuffer);

private:
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;
    bool connected;

    int _startDAC;
    int _endDAC;
    int _startPort;
    int _endPort;
    int _startLevel;
    int _endLevel;
    int _incrementLevel;
    bool enabledModeling;
    bool enabledContinue;

    void run();
};

#endif // LEDDRIVER_H

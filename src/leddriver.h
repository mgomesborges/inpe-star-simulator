#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <QThread>
#include <Qtime>
#include <QVector>

#include <Windows.h>
#include "ftd2xx.h"

#include "ftdidevicechooserdialog.h"

class LedDriver : public QThread
{
    Q_OBJECT
public:
    enum modelingParameters {
        levelIncrement,
        levelDecrement
    };

    enum mode {
        ledModeling,
        channelTest
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
    void saveData(QString currentChannel);
    void modelingFinished();
    void testFinished();
    void info(QString message);

public slots:
    void stop();
    bool openConnection();
    void closeConnection();
    bool isConnected();
    bool updateChannel(int channel, int value);
    bool resetDACs();
    bool configureVoltage();
    void setV2Ref(bool enable);
    void enabledModelingContinue();
    bool ftdiCyclePort();
    int operationMode();
    void setOperationMode(int mode);
    int currentChannel();

private:
    void run();
    bool writeData(int dac, int port, int value);

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
    int _channel;
    int _operationMode;
    QVector<int> levelIndex;
};

#endif // LEDDRIVER_H

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

    bool openConnection();
    void closeConnection();
    bool writeData(const char *txBuffer);
    
signals:
    void connectionError(QString serialError, QString statusBarMsg);
    void transmissionError();
    
public slots:

private:
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;
    const char *data;

    void run();
};

#endif // LEDDRIVER_H

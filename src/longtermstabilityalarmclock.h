#ifndef LONGTERMSTABILITYALARMCLOCK_H
#define LONGTERMSTABILITYALARMCLOCK_H

#include <QThread>
#include <QTime>
#include <QVector>
#include <qmath.h>

class LongTermStabilityAlarmClock : public QThread
{
    Q_OBJECT
public:
    explicit LongTermStabilityAlarmClock(QObject *parent = 0);

    void setAlarmClock(int hour, int min, int sec, int timeIntervalInSec);

signals:
    void timeout();
    void finished();

public slots:
    void stop();

private:
    void run();

    bool stopThread;
    int milliSecondTimeInterval;
    QVector<int> milliSecondTimeToRun;
    QTime timeHandle;
};

#endif // LONGTERMSTABILITYALARMCLOCK_H

#ifndef LONGTERMSTABILITYALARMCLOCK_H
#define LONGTERMSTABILITYALARMCLOCK_H

#include <QThread>

class LongTermStabilityAlarmClock : public QThread
{
    Q_OBJECT
public:
    explicit LongTermStabilityAlarmClock(QObject *parent = 0);

    void setAlarmClock(int hours, int minutes, int seconds, int timeIntervalInSeconds);

signals:
    void timeout();
    void finished();

public slots:
    void stop();

private:
    void run();

    int timeInterval;
    int numberOfTimesToRun;
    bool stopThread;
};

#endif // LONGTERMSTABILITYALARMCLOCK_H

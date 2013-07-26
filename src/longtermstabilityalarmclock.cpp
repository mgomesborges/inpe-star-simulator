#include "longtermstabilityalarmclock.h"

LongTermStabilityAlarmClock::LongTermStabilityAlarmClock(QObject *parent) :
    QThread(parent)
{
}

void LongTermStabilityAlarmClock::setAlarmClock(int hour, int min, int sec, int timeIntervalInSec)
{
    // The counter wraps to zero 24 hours after the last call to start() or restart
    unsigned int timeInMilliSeconds = (hour * 60 * 60 * 1000) + (min * 60 * 1000) + (sec * 1000);
    int dayInMilliSeconds  = 24 * 60 * 60 * 1000;
    int numberOfIteration  = qCeil(double(timeInMilliSeconds) / double(dayInMilliSeconds));

    milliSecondTimeToRun.clear();
    for (int i = 0; i < (numberOfIteration - 1); i++) {
        milliSecondTimeToRun.append(dayInMilliSeconds);
        timeInMilliSeconds -= dayInMilliSeconds;
    }
    milliSecondTimeToRun.append(timeInMilliSeconds);

    milliSecondTimeInterval = timeIntervalInSec * 1000;
}

void LongTermStabilityAlarmClock::stop()
{
    stopThread = true;
}

void LongTermStabilityAlarmClock::run()
{
    stopThread = false;

    // Start timeHandle with current time
    timeHandle.start();

    for (int i = 0; i < milliSecondTimeToRun.size(); i++) {
        while (stopThread == false) {
            msleep(milliSecondTimeInterval);
            if (timeHandle.elapsed() > milliSecondTimeToRun[i]) {
                break;
            }
            emit timeout();
        }
    }

    emit finished();
}

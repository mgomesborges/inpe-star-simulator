#include "longtermstabilityalarmclock.h"

LongTermStabilityAlarmClock::LongTermStabilityAlarmClock(QObject *parent) :
    QThread(parent)
{
}

void LongTermStabilityAlarmClock::setAlarmClock(int hours, int minutes, int seconds, int timeIntervalInSeconds)
{
    int timeInSeconds  = (hours * 60 * 60) + (minutes * 60) + seconds;
    numberOfTimesToRun = timeInSeconds / timeIntervalInSeconds;
    timeInterval       = timeIntervalInSeconds;
}

void LongTermStabilityAlarmClock::stop()
{
    stopThread = true;
}

void LongTermStabilityAlarmClock::run()
{
    stopThread  = false;
    timeoutFlag = false;
    for (int i = 0; i < numberOfTimesToRun; i++) {
        msleep(timeInterval * 1000); // msleep more precise than sleep()
        emit timeout();
        timeoutFlag = true;
        if (stopThread == true) {
            break;
        }
    }
    emit finished();
}


void LongTermStabilityAlarmClock::clearTimeout()
{
    timeoutFlag = false;
}

bool LongTermStabilityAlarmClock::isTimeout()
{
    return timeoutFlag;
}

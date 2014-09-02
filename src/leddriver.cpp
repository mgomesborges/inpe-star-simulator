#include "leddriver.h"

LedDriver::LedDriver(QObject *parent) :
    QThread(parent)
{
    connected      = false;
    v2ref          = true;
    mode = ledModeling;
}

LedDriver::~LedDriver()
{
    closeConnection();
}

void LedDriver::stop()
{
    enabledModeling = false;
}

bool LedDriver::openConnection()
{
    FTDIDeviceChooserDialog ftdiDevice;
    int iDevice = ftdiDevice.defaultConnection();

    if (iDevice >= 0) {
        for (int i = 0; i < 5; i++) {
            if (FT_Open(iDevice, &ftHandle) == FT_OK) {
                FT_SetTimeouts(ftHandle, 1000, 1000);
                connected = true;
                if (configureVoltage() == false)
                    closeConnection();

                return connected;
            }
            msleep(1000);
        }
    }

    connected = false;
    return false;
}

void LedDriver::closeConnection()
{
    if (isConnected() == true)
        resetDACs();

    connected = false;
    FT_Close(ftHandle);
}

bool LedDriver::isConnected()
{
    return connected;
}

bool LedDriver::updateChannel(int channel, int value)
{
    int dac;
    int port;

    // Find the value of DAC
    if ((channel % 8) != 0)
        dac  = (channel / 8);
    else
        dac  = channel / 8 - 1;

    // Find the value of Port
    port = channel - (dac * 8) - 1;

    return writeData(dac, port, value);
}

bool LedDriver::writeData(int dac, int port, int value)
{
    if (connected == true) {
        DWORD bytesWritten;
        char txBuffer[9];

        txBuffer[0] = 0x0C;
        txBuffer[1] = 0x40 |  (value & 0x0000000F);
        txBuffer[2] = 0x80 | ((value & 0x000000F0) >> 4);
        txBuffer[3] = 0x0D;
        txBuffer[4] = 0x40 | ((value & 0x00000F00) >> 8);
        txBuffer[5] = 0x80 | port;
        txBuffer[6] =  dac;
        txBuffer[7] = 0x40;
        txBuffer[8] = 0x80;

        for (int i = 0; i < 9; i++) {
            for (int attempts = 0; attempts < 3; attempts++) {
                if (FT_Write(ftHandle, &txBuffer[i], 1, &bytesWritten) == FT_OK && bytesWritten == 1)
                    break; // Ok, next information

                if (FT_ResetDevice(ftHandle) == FT_OK)
                    FT_SetTimeouts(ftHandle, 1000, 1000);
                else
                    return false;
            }
        }

        return true;
    }

    return false;
}

bool LedDriver::resetDACs()
{
    // RESET DAC0 to DAC11
    for (int dac = 0; dac < 12; dac++) {
        if (writeData(dac, 0xF, 0) == false)
            return false;
    }

    return configureVoltage();
}

bool LedDriver::configureVoltage()
{
    int vref = 0x30; // v2ref ON

    if (v2ref == false)
        vref = 0x00; // v2ref OFF

    for (int dac = 0; dac < 12; dac++)
        if (writeData(dac, 0x8, vref) == false)
            return false;

    return true;
}

void LedDriver::setV2Ref(bool enable)
{
    v2ref = enable;
    configureVoltage();
}

void LedDriver::setModelingParameters(int startChannelValue, int endChannelValue, int levelUpdateType, int incrementDecrementValue)
{
    startChannel = startChannelValue;
    endChannel   = endChannelValue;
    updateType   = levelUpdateType;
    incDecValue  = incrementDecrementValue;
}

void LedDriver::enabledModelingContinue()
{
    enabledContinue = true;
}

/**
 * @brief The effect of this function is the same as disconnecting then reconnecting the device from USB.
 * @return True if Cycle Port is performed, otherwise return false.
 */
bool LedDriver::ftdiCyclePort()
{
    FTDIDeviceChooserDialog ftdiDevice;

    for (int attempts = 0; attempts < 10; attempts++) {
        if (FT_CyclePort(ftHandle) == FT_OK) {
            for (attempts = 0; attempts < 30; attempts++) {
                msleep(2000);
                if (ftdiDevice.defaultConnection(false) != -1)
                    return openConnection();
            }
            return false;
        }
        msleep(500);
    }
    return false;
}

int LedDriver::operationMode()
{
    return mode;
}

void LedDriver::setOperationMode(int mode)
{
    this->mode = mode;
}

void LedDriver::setActiveChannels(const QVector<int> &activeChannels)
{
    this->activeChannels = activeChannels;
}

int LedDriver::currentChannel()
{
    return channel;
}

int LedDriver::currentLevel()
{
    return level;
}

void LedDriver::modelingNextChannel()
{
    nextChannel = true;
}

QVector<int> LedDriver::digitalLevelIndex()
{
    return levelIndex;
}

void LedDriver::run()
{
    if (mode == ledModeling) {
        QTime timer;
        int dac;
        int port;

        enabledModeling = true;
        nextChannel     = false;

        for (channel = startChannel; channel <= endChannel && enabledModeling == true; channel++) {
            if (activeChannels[channel - 1] == 1) {
                resetDACs();

                // Setup Increment or Decrement level
                level = 0;

                if (updateType == levelDecrement)
                    level  = 4095;

                levelIndex.clear();

                for (int counter = 0; counter < 4096; counter++) {
                    levelIndex.append(level);

                    // Find the value of DAC
                    if ((channel % 8) != 0)
                        dac  = (channel / 8);
                    else
                        dac  = channel / 8 - 1;

                    // Find the value of Port
                    port = channel - (dac * 8) - 1;

                    if (writeData(dac, port, level) == false) {
                        emit warningMessage(tr("LED Driver Error"), tr("Writes Data Error.\nCheck USB connection and try again!"));
                        emit modelingFinished();
                        resetDACs();
                        return;
                    }

                    // Performs Scan with SMS500
                    enabledContinue = false;
                    emit performScan();
                    timer.start();

                    while (enabledContinue == false) {
                        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
                        if (timer.elapsed() >= 600000) {
                            emit warningMessage(tr("LED Driver Error"), tr("Wait time to SMS500 perform scan or save data exceeded."));
                            emit modelingFinished();
                            resetDACs();
                            return;
                        }
                        if (enabledModeling == false) {
                            emit modelingFinished();
                            resetDACs();
                            return;
                        }
                    }

                    if (nextChannel == true) {
                        nextChannel = false;
                        break; // Go to Next Channel
                    }

                    // Computing next level
                    if (updateType == levelIncrement) {
                        level += incDecValue;
                        if (level > 4095)
                            break; // Go to Next Channel
                    } else if (updateType == levelDecrement) {
                        level -= incDecValue;
                        if (level < 0 )
                            break; // Go to Next Channel
                    }
                }

                emit saveData(QString::number(channel));

                // Cycle Port
                if (ftdiCyclePort() == false) {
                    emit warningMessage(tr("LED Driver Error :: Cycle Port"), tr("Reset Device Error.\nCheck USB connection and try again!"));
                    break;
                }
            } else {
                emit saveData(QString::number(channel));
            }
        }

        emit modelingFinished();
        resetDACs();
    }

    else if (mode == channelTest) {
        int dac;
        int port;

        enabledModeling = true;

        for (channel = 1; channel <= 96; channel++) {
            resetDACs();

            // Find the value of DAC
            if ((channel % 8) != 0)
                dac  = (channel / 8);
            else
                dac  = channel / 8 - 1;

            // Find the value of Port
            port = channel - (dac * 8) - 1;

            if (writeData(dac, port, 4095) == false) {
                emit warningMessage(tr("LED Driver Error"), tr("Writes Data Error.\nCheck USB connection and try again!"));
                emit testFinished();
                return;
            }

            // Performs Scan with SMS500
            enabledContinue = false;
            emit performScan();

            while (enabledContinue == false && enabledModeling == true)
                msleep(1); // wait 1ms for continue, see Qt Thread's Documentation

            if (enabledModeling == false)
                break;
        }

        resetDACs();
        emit testFinished();
    }
}

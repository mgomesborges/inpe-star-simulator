#include "leddriver.h"

LedDriver::LedDriver(QObject *parent) :
    QThread(parent)
{
    connected = false;
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
    if (FT_Open(0, &ftHandle) == FT_OK) {

        FT_SetTimeouts(ftHandle, 1000, 1000);

        // Set 2Vref
        char txBuffer[10];
        for (int dac = 0; dac < 12; dac++) {
            txBuffer[0] = 0x0C;
            txBuffer[1] = 0x40;
            txBuffer[2] = 0x83;
            txBuffer[3] = 0x0D;
            txBuffer[4] = 0x40;
            txBuffer[5] = 0x88;
            txBuffer[6] =  dac;
            txBuffer[7] = 0x40;
            txBuffer[8] = 0x80;
            txBuffer[9] = '\0';

            writeData(txBuffer);

            connected = true;
        }
    } else {
        connected = false;
    }

    return connected;
}

void LedDriver::closeConnection()
{
    wait(1000);

    if (isConnected() == true) {
        resetDACs();
    }

    FT_Close(ftHandle);

    connected = false;
}

bool LedDriver::isConnected()
{
    return connected;
}

void LedDriver::enabledModelingContinue()
{
    enabledContinue = true;
}

bool LedDriver::writeData(const char *txBuffer)
{
    DWORD bytesWritten;
    char dataToConstChar;
    QTime timer;

    int length = QString( txBuffer ).length();

    for (int i = 0; i < length; i++) {
        dataToConstChar = txBuffer[i];

        timer.restart();
        ftStatus = FT_Write(ftHandle, &dataToConstChar, 1, &bytesWritten);

        // Checks if timeout has occurred
        if (timer.elapsed() >= 1000) {
            ftStatus = FT_ResetDevice(ftHandle); // Reset FTDI

            if (ftStatus == FT_OK) { // If Reset Ok, Sets Timeouts and Writes Data again
                FT_SetTimeouts(ftHandle, 1000, 1000);
                ftStatus = FT_Write(ftHandle, &dataToConstChar, 1, &bytesWritten);
            }
            else {
                emit warningMessage(tr("LED Driver Error"), tr("Reset Device Error.\nCheck USB connection and try again!"));
                return false;
            }
        }

        if ((ftStatus != FT_OK) || (bytesWritten != 1)) {
            return false;
        }
    }

    return true;
}

bool LedDriver::resetDACs()
{
    char txBuffer[10];

    // RESET DAC0 to DAC11
    for (int i = 0; i < 12; i++) {
        txBuffer[0] = 0x0C;
        txBuffer[1] = 0x40;
        txBuffer[2] = 0x80;
        txBuffer[3] = 0x0D;
        txBuffer[4] = 0x40;
        txBuffer[5] = 0x8F;
        txBuffer[6] =    i;
        txBuffer[7] = 0x40;
        txBuffer[8] = 0x80;
        txBuffer[9] = '\0';

        if (writeData(txBuffer) == false) {
            return false;
        }
    }

    // Set 2Vref
    for (int dac = 0; dac < 12; dac++) {
        txBuffer[0] = 0x0C;
        txBuffer[1] = 0x40;
        txBuffer[2] = 0x83;
        txBuffer[3] = 0x0D;
        txBuffer[4] = 0x40;
        txBuffer[5] = 0x88;
        txBuffer[6] =  dac;
        txBuffer[7] = 0x40;
        txBuffer[8] = 0x80;
        txBuffer[9] = '\0';

        if (writeData(txBuffer) == false) {
            return false;
        }
    }

    return true;
}

void LedDriver::setModelingParameters(int startChannelValue, int endChannelValue, int levelDecrementValue)
{
    startChannel   = startChannelValue;
    endChannel     = endChannelValue;
    levelDecrement = levelDecrementValue;
}

void LedDriver::modelingNextChannel()
{
    nextChannel = true;
}

void LedDriver::run()
{
    char txBuffer[10];
    DWORD bytesWritten;
    QTime timer;
    int dac;
    int port;

    enabledModeling = true;
    nextChannel     = false;

    for (int channel = startChannel; channel <= endChannel; channel++) {

        resetDACs();

        for (int level = 4095; level > 0; level -= levelDecrement) {
            // Find the value of DAC
            if ((channel % 8) != 0) {
                dac  = (channel / 8);
            } else {
                dac  = channel / 8 - 1;
            }

            // Find the value of Port
            port = channel - (dac * 8) - 1;

            // LED Driver configure: set DAC and Port value
            txBuffer[0] = 0x0C;
            txBuffer[1] = 0x40 |  (level & 0x0000000F);
            txBuffer[2] = 0x80 | ((level & 0x000000F0) >> 4);
            txBuffer[3] = 0x0D;
            txBuffer[4] = 0x40 | ((level & 0x00000F00) >> 8);
            txBuffer[5] = 0x80 | port;
            txBuffer[6] = dac;
            txBuffer[7] = 0x40;
            txBuffer[8] = 0x80;
            txBuffer[9] = '\0';

            // Writes Data
            for (int i = 0; i < 9; i++) {
                timer.restart();

                ftStatus = FT_Write(ftHandle, &txBuffer[i], 1, &bytesWritten);

                // Checks if timeout has occurred
                if (timer.elapsed() >= 1000) {
                    ftStatus = FT_ResetDevice(ftHandle); // Reset FTDI

                    if (ftStatus == FT_OK) { // If Reset Ok, Sets Timeouts and Writes Data again
                        FT_SetTimeouts(ftHandle, 1000, 1000);
                        ftStatus = FT_Write(ftHandle, &txBuffer[i], 1, &bytesWritten);
                    }
                    else {
                        emit warningMessage(tr("LED Driver Error"), tr("Reset Device Error.\nCheck USB connection and try again!"));
                        emit modelingFinished();
                        return;
                    }
                }

                if ((ftStatus != FT_OK) || (bytesWritten != 1)) {
                    emit warningMessage(tr("LED Driver Error"), tr("Writes Data Error.\nCheck USB connection and try again!"));
                    emit modelingFinished();
                    return;
                }
            }

            // Performs Scan with SMS500
            enabledContinue = false;

            emit performScan();

            // Waiting for SMS500 be ready for next scan
            timer.restart();
            while ((enabledModeling == true) && (enabledContinue == false)) {
                if (timer.elapsed() >= 600000) {
                    emit warningMessage(tr("LED Driver Error"), tr("Wait Time Exceeded.\nWainting for SMS500 be read for next Scan."));
                    emit modelingFinished();
                    stop();
                    return;
                }
            }

            emit info(tr("[Channel %1, Digital Level %2]").arg(channel).arg(level));

            // Save Data in .txt format
            enabledContinue = false;
            emit saveData(tr("/ch%1_DigitalLevel_%2.txt").arg(channel).arg(level));

            // Waiting Save Data
            timer.restart();
            while ((enabledModeling == true) && (enabledContinue == false)) {
                if (timer.elapsed() >= 5000) {
                    emit warningMessage(tr("LED Driver Error"), tr("Wait Time Exceeded.\nWaiting for Save Data"));
                    emit modelingFinished();
                    stop();
                    return;
                }
            }

            // Ends LED Modeling
            if( (enabledModeling == false) || (connected == false)) {
                emit modelingFinished();
                return;
            }

            if (nextChannel == true) {
                nextChannel = false;
                break; // Go to Next Channel
            }
        }

        emit saveAllData(QString::number(channel));

        // Cycle Port
        // The effect of this function is the same as disconnecting then reconnecting the device from USB.
        ftStatus = FT_CyclePort(ftHandle);
        if (ftStatus != FT_OK) {
            // Try again
            ftStatus = FT_CyclePort(ftHandle);
            if (ftStatus != FT_OK) {
                emit warningMessage(tr("LED Driver Error :: Cycle Port"), tr("Cycle Port Error."));
                emit modelingFinished();
                return;
            }
        }

        // Waites FTDI ready
        sleep(20);

        if (FT_Open(0, &ftHandle) != FT_OK) {
            // Try again
            if (FT_Open(0, &ftHandle) != FT_OK) {
                emit warningMessage(tr("LED Driver Error :: Cycle Port"), tr("Reset Device Error.\nCheck USB connection and try again!"));
                emit modelingFinished();
                return;
            }
        }

        FT_SetTimeouts(ftHandle, 1000, 1000);
    }

    emit modelingFinished();
}

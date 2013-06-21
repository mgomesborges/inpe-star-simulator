#include "leddriver.h"

#include <QDebug>

#include <Qtime>

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

            writeData(txBuffer);
        }
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

    int length = QString( txBuffer ).length();

    for (int i = 0; i < length; i++) {
        dataToConstChar = txBuffer[i];

        ftStatus = FT_Write(ftHandle, &dataToConstChar, 1, &bytesWritten);

        if ((ftStatus != FT_OK) || (bytesWritten != 1)) {
            return false;
        }
    }

    return true;
}

void LedDriver::setModelingParameters(int startDAC, int endDAC, int startPort, int endPort,
                                      int startLevel, int endLevel, int incrementLevel)
{
    _startDAC       = startDAC;
    _endDAC         = endDAC;
    _startPort      = startPort;
    _endPort        = endPort;
    _startLevel     = startLevel;
    _endLevel       = endLevel;
    _incrementLevel = incrementLevel;
}

void LedDriver::run()
{
    char txBuffer[10];
    DWORD bytesWritten;
    QTime timer;

    enabledModeling = true;

    for (; _startDAC <= _endDAC; _startDAC++) {
        for (; _startPort <= _endPort; _startPort++) {
            for (; _startLevel >= _endLevel; _startLevel -= _incrementLevel) {

                // LED Driver configure: set DAC and Port value
                txBuffer[0] = 0x0C;
                txBuffer[1] = 0x40 |  (_startLevel & 0x0000000F);
                txBuffer[2] = 0x80 | ((_startLevel & 0x000000F0) >> 4);
                txBuffer[3] = 0x0D;
                txBuffer[4] = 0x40 | ((_startLevel & 0x00000F00) >> 8);
                txBuffer[5] = 0x80 | _startPort;
                txBuffer[6] = _startDAC;
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
                            return;
                        }
                    }

                    if ((ftStatus != FT_OK) || (bytesWritten != 1)) {
                        emit warningMessage(tr("LED Driver Error"), tr("Writes Data Error.\nCheck USB connection and try again!"));
                        return;
                    }
                }

                // Performs Scan with SMS500
                enabledContinue = false;

                emit performScan();

                // Waiting for SMS500 be ready for next scan
                timer.restart();
                while ((enabledModeling == true) && (enabledContinue == false)) {
                    if (timer.elapsed() >= 20000) {
                        emit warningMessage(tr("LED Driver Error"), tr("Wait Time Exceeded.\nWainting for SMS500 be read for next Scan."));
                    }
                }

                // Save Data in .txt format
                enabledContinue = false;
                emit saveData(tr("/channel%1_%2nd.txt")
                              .arg(_startDAC * 8 + _startPort + 1)
                              .arg(_startLevel));

                // Waiting Save Data
                timer.restart();
                while ((enabledModeling == true) && (enabledContinue == false)) {
                    if (timer.elapsed() >= 5000) {
                        emit warningMessage(tr("LED Driver Error"), tr("Wait Time Exceeded.\nWaiting for Save Data"));
                    }
                }

                // Ends LED Modeling
                if( (enabledModeling == false) || (connected == false)) {
                    emit modelingFinished();
                    return;
                }
            }
        }
    }

    emit modelingFinished();
}

#include "leddriver.h"

LedDriver::LedDriver(QObject *parent) :
    QThread(parent)
{
}

LedDriver::~LedDriver()
{
    closeConnection();
}

bool LedDriver::openConnection()
{
    if (FT_Open(0, &ftHandle) == FT_OK) {
        if ((FT_SetBaudRate(ftHandle, FT_BAUD_19200) &&
             FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE) &&
             FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x11, 0x13)) == FT_OK) {

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
                wait();

                return true;
            }
        } else {
            emit connectionError(tr("Can't opened the serial port."), tr("LED Driver Serial Port Configure error"));
        }
    } else {
        emit connectionError(tr("Can't configure the serial port."), tr("LED Driver Open Comunication Error"));
    }

    return false;
}

void LedDriver::closeConnection()
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

        writeData(txBuffer);
        wait();
    }

    FT_Close(ftHandle);
}

bool LedDriver::writeData(const char *txBuffer)
{
    data = txBuffer;

    run();

    if (ftStatus == FT_OK) {
        return true;
    }
    return false;
}

void LedDriver::run()
{
    DWORD bytesWritten;
    char dataToConstChar;
    int length = QString( data ).length();

    for (int i = 0; i < length; i++) {
        dataToConstChar = data[i];

        ftStatus = FT_Write(ftHandle, &dataToConstChar, 1, &bytesWritten);
        if (ftStatus != FT_OK) {
            break;
        }
    }
}

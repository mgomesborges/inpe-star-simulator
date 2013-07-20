#include "lsqloadleddata.h"

LSqLoadLedData::LSqLoadLedData(QObject *parent, QString path) :
    QThread(parent)
{
    inputPath = path;
}

void LSqLoadLedData::stop()
{
    stopThread = true;
}

void LSqLoadLedData::run()
{
    int numberOfRows;
    int numberOfColumns;

    // Derivative Step is negative because the digital level is from 4095 to 0
    signed int derivativeStep = -1;

    stopThread = false;

    // Computing Derivatives for all channels
    for (int channel = 25; channel <= 96; channel++) {
        emit progressInfo(channel);
        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        if (stopThread == true) {
            return;
        }

        // Opens input data in txt file
        QString filePath = inputPath + "/ch" + QString::number(channel) + ".txt";
        QFile file(filePath);
        if (!file.exists()) {
            emit error(tr("Error: LED Data not found!"));
            return;
        }
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString line       = in.readLine();
        QStringList fields = line.split("\t");
        numberOfRows       = 641;
        numberOfColumns    = fields.size();

        // Define matrix of floats, see Eigen Matrix documentation
        MatrixXd matrix(numberOfRows, numberOfColumns);

        // Reposition to beginning of file
        in.seek(0);
        for (int i = 0; i < numberOfRows; i++) {
            line   = in.readLine();
            fields = line.split("\t");
            for (int j = 0; j < numberOfColumns; j++) {
                matrix(i,j) = fields.at(j).toDouble();
            }
        }
        file.close();

        // Computing Derivatives
        MatrixXd derivatives(numberOfRows, numberOfColumns-1);
        derivatives.col(0) = matrix.col(0); // Column 0 contains wavelengths values
        for (int column = 1; column < numberOfColumns - 1; column++) {
            derivatives.col(column) = (matrix.col(column+1) - matrix.col(column)) / derivativeStep;
        }

        // Saves output data in binary file
        QString outFilePath = QDir::currentPath() + "/led_database/ch" + QString::number(channel) + ".led";
        QFile outFile(outFilePath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            emit error(tr("Error: can not create database!"));
            return;
        }
        QDataStream out(&outFile);
        out.setVersion(QDataStream::Qt_5_0);
        out << numberOfRows;
        out << numberOfColumns - 1;
        for (int i = 0; i < numberOfRows; i++) {
            for (int j = 0; j < numberOfColumns - 1; j++) {
                out << derivatives(i,j);
            }
        }
        outFile.close();
    }
}
